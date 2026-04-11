#!/usr/bin/env python3

from __future__ import annotations

import argparse
import html
import json
import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import parse_qsl, urlencode, urlparse, urlunparse


EDGE_BIN = "/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge"
DEFAULT_VIRTUAL_TIME_BUDGET_MS = 4000


def resolve_front_edge_viewport() -> tuple[int, int]:
    bounds_raw = subprocess.check_output(
        [
            "osascript",
            "-e",
            'tell application "Microsoft Edge" to get bounds of front window',
        ],
        text=True,
    ).strip()
    left, top, right, bottom = [int(part.strip()) for part in bounds_raw.split(",")]
    return right - left, bottom - top


def build_probe_url(source_url: str, node_id: str | None) -> str:
    parsed = urlparse(source_url)
    params = dict(parse_qsl(parsed.query, keep_blank_values=True))
    params["probe"] = "geometry-text"
    if node_id:
        params["probeNodeId"] = node_id
    return urlunparse(parsed._replace(query=urlencode(params)))


def extract_probe_payload(dom: str) -> str:
    match = re.search(
        r'<pre[^>]*data-geometry-text-probe="ready"[^>]*>(.*?)</pre>',
        dom,
        re.DOTALL,
    )
    if not match:
        raise RuntimeError("Browser probe payload was not ready in dumped DOM")
    return html.unescape(match.group(1)).strip()


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Measure real browser-side geometry text padding via a hidden DOM probe.",
    )
    parser.add_argument("--url", required=True, help="SlideApp URL to probe")
    parser.add_argument("--node-id", help="Optional node id to measure")
    parser.add_argument("--viewport", help="Viewport as WIDTHxHEIGHT; defaults to front Edge window")
    parser.add_argument("--out", help="Optional file path for the JSON probe payload")
    parser.add_argument(
        "--virtual-time-budget-ms",
        type=int,
        default=DEFAULT_VIRTUAL_TIME_BUDGET_MS,
    )
    args = parser.parse_args()

    edge_bin = Path(EDGE_BIN)
    if not edge_bin.exists():
        raise SystemExit(f"Missing Microsoft Edge binary at: {edge_bin}")

    if args.viewport:
        viewport_raw = args.viewport.lower()
        viewport_width, viewport_height = [int(part) for part in viewport_raw.split("x", 1)]
    else:
        viewport_width, viewport_height = resolve_front_edge_viewport()

    probe_url = build_probe_url(args.url, args.node_id)
    dom = subprocess.check_output(
        [
            str(edge_bin),
            "--headless=new",
            "--disable-gpu",
            "--hide-scrollbars",
            f"--window-size={viewport_width},{viewport_height}",
            f"--virtual-time-budget={args.virtual_time_budget_ms}",
            "--dump-dom",
            probe_url,
        ],
        text=True,
    )
    payload = extract_probe_payload(dom)
    parsed = json.loads(payload)
    formatted = json.dumps(parsed, ensure_ascii=False, indent=2)

    if args.out:
        out_path = Path(args.out).expanduser().resolve()
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(formatted + "\n", encoding="utf-8")

    sys.stdout.write(formatted + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
