#!/usr/bin/env python3

from __future__ import annotations

import argparse
import html
import json
import math
import shutil
import subprocess
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parents[1]
DRAWIO_DIR = REPO_ROOT / "Docs" / "剧本" / "图"
DOCS_PREVIEW_DIR = REPO_ROOT / ".generated" / "storyboard-previews" / "docs"
WEB_PREVIEW_DIR = REPO_ROOT / "SlideApp" / "public" / "storyboard-diagrams"
MANIFEST_PATH = WEB_PREVIEW_DIR / "manifest.json"
SKIP_DRAFT_STEMS = {"01-封面"}

BG_COLOR = "#FCFBF7"
GRID_COLOR = "#ECE8E1"
TEXT_FONT = "ui-sans-serif, -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif"


@dataclass(frozen=True)
class RectNode:
    cell_id: str
    label: str
    x: float
    y: float
    width: float
    height: float
    fill: str
    stroke: str
    text: str
    stroke_width: float
    font_size: float
    dashed: bool

    @property
    def center_x(self) -> float:
        return self.x + self.width / 2

    @property
    def center_y(self) -> float:
        return self.y + self.height / 2


@dataclass(frozen=True)
class Edge:
    source_id: str
    target_id: str
    stroke: str
    stroke_width: float
    dashed: bool
    exit_x: float | None
    exit_y: float | None
    entry_x: float | None
    entry_y: float | None


def parse_style(style: str | None) -> dict[str, str]:
    if not style:
        return {}
    result: dict[str, str] = {}
    for item in style.split(";"):
        if not item or "=" not in item:
            continue
        key, value = item.split("=", 1)
        result[key] = value
    return result


def float_or_none(value: str | None) -> float | None:
    if value is None or value == "":
        return None
    return float(value)


def parse_drawio(path: Path) -> tuple[int, int, list[RectNode], list[Edge]]:
    tree = ET.parse(path)
    graph = tree.find(".//mxGraphModel")
    if graph is None:
        raise ValueError(f"mxGraphModel missing in {path}")
    page_width = int(graph.attrib.get("pageWidth", "1600"))
    page_height = int(graph.attrib.get("pageHeight", "900"))

    nodes: list[RectNode] = []
    edges: list[Edge] = []

    for cell in graph.findall(".//mxCell"):
        style = parse_style(cell.attrib.get("style"))
        if cell.attrib.get("vertex") == "1":
            geometry = cell.find("mxGeometry")
            if geometry is None:
                continue
            fill = style.get("fillColor", "#F6F1E8")
            nodes.append(
                RectNode(
                    cell_id=cell.attrib["id"],
                    label=cell.attrib.get("value", ""),
                    x=float(geometry.attrib.get("x", "0")),
                    y=float(geometry.attrib.get("y", "0")),
                    width=float(geometry.attrib.get("width", "0")),
                    height=float(geometry.attrib.get("height", "0")),
                    fill=fill,
                    stroke=style.get("strokeColor", "#7B8794"),
                    text=style.get("fontColor", "#26313D"),
                    stroke_width=float(style.get("strokeWidth", "2")),
                    font_size=float(style.get("fontSize", "20")),
                    dashed=style.get("dashed") == "1",
                )
            )
        elif cell.attrib.get("edge") == "1":
            edges.append(
                Edge(
                    source_id=cell.attrib.get("source", ""),
                    target_id=cell.attrib.get("target", ""),
                    stroke=style.get("strokeColor", "#7B8794"),
                    stroke_width=float(style.get("strokeWidth", "1.8")),
                    dashed=style.get("dashed") == "1",
                    exit_x=float_or_none(style.get("exitX")),
                    exit_y=float_or_none(style.get("exitY")),
                    entry_x=float_or_none(style.get("entryX")),
                    entry_y=float_or_none(style.get("entryY")),
                )
            )

    return page_width, page_height, nodes, edges


def rect_anchor(node: RectNode, frac_x: float, frac_y: float) -> tuple[float, float]:
    return (node.x + node.width * frac_x, node.y + node.height * frac_y)


def auto_source_anchor(source: RectNode, target: RectNode) -> tuple[float, float]:
    dx = target.center_x - source.center_x
    dy = target.center_y - source.center_y
    if abs(dx) >= abs(dy):
        return (
            rect_anchor(source, 1, 0.5)
            if dx >= 0
            else rect_anchor(source, 0, 0.5)
        )
    return (
        rect_anchor(source, 0.5, 1)
        if dy >= 0
        else rect_anchor(source, 0.5, 0)
    )


def auto_target_anchor(source: RectNode, target: RectNode) -> tuple[float, float]:
    dx = target.center_x - source.center_x
    dy = target.center_y - source.center_y
    if abs(dx) >= abs(dy):
        return (
            rect_anchor(target, 0, 0.5)
            if dx >= 0
            else rect_anchor(target, 1, 0.5)
        )
    return (
        rect_anchor(target, 0.5, 0)
        if dy >= 0
        else rect_anchor(target, 0.5, 1)
    )


def side_from_anchor(node: RectNode, point: tuple[float, float]) -> str:
    x, y = point
    if math.isclose(x, node.x, abs_tol=0.1):
        return "left"
    if math.isclose(x, node.x + node.width, abs_tol=0.1):
        return "right"
    if math.isclose(y, node.y, abs_tol=0.1):
        return "top"
    return "bottom"


def orthogonal_path(
    start: tuple[float, float],
    end: tuple[float, float],
    start_side: str,
    end_side: str,
) -> list[tuple[float, float]]:
    points = [start]

    if start_side in {"left", "right"} and end_side in {"left", "right"}:
        mid = (end[0], start[1])
        if not almost_same(mid, start) and not almost_same(mid, end):
            points.append(mid)
    elif start_side in {"top", "bottom"} and end_side in {"top", "bottom"}:
        mid = (start[0], end[1])
        if not almost_same(mid, start) and not almost_same(mid, end):
            points.append(mid)
    elif start_side in {"top", "bottom"}:
        mid = (start[0], end[1])
        if not almost_same(mid, start) and not almost_same(mid, end):
            points.append(mid)
    else:
        mid = (end[0], start[1])
        if not almost_same(mid, start) and not almost_same(mid, end):
            points.append(mid)

    if not almost_same(points[-1], end):
        points.append(end)
    return dedupe_points(points)


def almost_same(a: tuple[float, float], b: tuple[float, float]) -> bool:
    return math.isclose(a[0], b[0], abs_tol=0.1) and math.isclose(
        a[1], b[1], abs_tol=0.1
    )


def dedupe_points(points: Iterable[tuple[float, float]]) -> list[tuple[float, float]]:
    result: list[tuple[float, float]] = []
    for point in points:
        if not result or not almost_same(result[-1], point):
            result.append(point)
    return result


def marker_id_for(stroke: str) -> str:
    return "arrow-" + stroke.replace("#", "").lower()


def render_svg(
    width: int,
    height: int,
    nodes: list[RectNode],
    edges: list[Edge],
) -> str:
    node_map = {node.cell_id: node for node in nodes}
    stroke_palette = sorted({edge.stroke for edge in edges})
    marker_defs = "\n".join(
        f"""    <marker id="{marker_id_for(color)}" markerWidth="12" markerHeight="12" refX="9" refY="6" orient="auto" markerUnits="strokeWidth">
      <path d="M0,0 L12,6 L0,12 z" fill="{color}" />
    </marker>"""
        for color in stroke_palette
    )

    edge_parts: list[str] = []
    for edge in edges:
        source = node_map.get(edge.source_id)
        target = node_map.get(edge.target_id)
        if source is None or target is None:
            continue
        start = (
            rect_anchor(source, edge.exit_x, edge.exit_y)
            if edge.exit_x is not None and edge.exit_y is not None
            else auto_source_anchor(source, target)
        )
        end = (
            rect_anchor(target, edge.entry_x, edge.entry_y)
            if edge.entry_x is not None and edge.entry_y is not None
            else auto_target_anchor(source, target)
        )
        start_side = side_from_anchor(source, start)
        end_side = side_from_anchor(target, end)
        points = orthogonal_path(start, end, start_side, end_side)
        point_string = " ".join(f"{x:.1f},{y:.1f}" for x, y in points)
        dash = ' stroke-dasharray="8 7"' if edge.dashed else ""
        edge_parts.append(
            f'  <polyline points="{point_string}" fill="none" stroke="{edge.stroke}" '
            f'stroke-width="{edge.stroke_width:.1f}" stroke-linecap="round" '
            f'stroke-linejoin="round"{dash} marker-end="url(#{marker_id_for(edge.stroke)})" />'
        )

    node_parts: list[str] = []
    for node in nodes:
        dash = ' stroke-dasharray="8 7"' if node.dashed else ""
        radius = min(node.height / 2, 28)
        node_parts.append(
            f'  <rect x="{node.x:.1f}" y="{node.y:.1f}" width="{node.width:.1f}" '
            f'height="{node.height:.1f}" rx="{radius:.1f}" fill="{node.fill}" '
            f'stroke="{node.stroke}" stroke-width="{node.stroke_width:.1f}"{dash} />'
        )
        label = html.escape(node.label)
        baseline_y = node.y + node.height / 2 + node.font_size * 0.34
        node_parts.append(
            f'  <text x="{node.center_x:.1f}" y="{baseline_y:.1f}" '
            f'font-family="{TEXT_FONT}" font-size="{node.font_size:.1f}" '
            f'font-weight="600" fill="{node.text}" text-anchor="middle">{label}</text>'
        )

    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">
  <defs>
{marker_defs}
    <pattern id="grid" width="20" height="20" patternUnits="userSpaceOnUse">
      <path d="M20 0H0V20" fill="none" stroke="{GRID_COLOR}" stroke-width="1"/>
    </pattern>
  </defs>
  <rect width="{width}" height="{height}" fill="{BG_COLOR}" />
  <rect width="{width}" height="{height}" fill="url(#grid)" opacity="0.85" />
{chr(10).join(edge_parts)}
{chr(10).join(node_parts)}
</svg>
"""


def render_file(drawio_path: Path, docs_output: Path, web_output: Path, png: bool) -> dict[str, str]:
    width, height, nodes, edges = parse_drawio(drawio_path)
    svg = render_svg(width, height, nodes, edges)

    docs_svg_path = docs_output / f"{drawio_path.stem}.svg"
    web_svg_path = web_output / f"{drawio_path.stem}.svg"
    docs_svg_path.write_text(svg, encoding="utf-8")
    web_svg_path.write_text(svg, encoding="utf-8")

    if png and shutil.which("sips"):
        docs_png_path = docs_output / f"{drawio_path.stem}.png"
        subprocess.run(
            ["sips", "-s", "format", "png", str(docs_svg_path), "--out", str(docs_png_path)],
            check=True,
            capture_output=True,
            text=True,
        )

    return {
        "drawio": str(drawio_path.relative_to(REPO_ROOT)),
        "svg": f"/storyboard-diagrams/{drawio_path.stem}.svg",
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--only",
        action="append",
        default=[],
        help="Render only matching drawio stems, e.g. 04-OpenGL",
    )
    parser.add_argument(
        "--no-png",
        action="store_true",
        help="Skip PNG generation even if sips is available",
    )
    args = parser.parse_args()

    DOCS_PREVIEW_DIR.mkdir(parents=True, exist_ok=True)
    WEB_PREVIEW_DIR.mkdir(parents=True, exist_ok=True)

    selected = set(args.only)
    manifest: dict[str, dict[str, str]] = {}
    for drawio_path in sorted(DRAWIO_DIR.glob("*.drawio")):
        if drawio_path.stem in SKIP_DRAFT_STEMS:
            continue
        if selected and drawio_path.stem not in selected:
            continue
        entry = render_file(
            drawio_path,
            docs_output=DOCS_PREVIEW_DIR,
            web_output=WEB_PREVIEW_DIR,
            png=not args.no_png,
        )
        manifest[drawio_path.stem] = entry
        print(f"rendered {drawio_path.stem}")

    if manifest:
        MANIFEST_PATH.write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )


if __name__ == "__main__":
    main()
