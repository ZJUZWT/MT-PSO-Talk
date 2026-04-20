#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


EDGE_APPLESCRIPT = [
    "-e",
    "on run argv",
    "-e",
    'tell application "Microsoft Edge"',
    "-e",
    "set js to item 1 of argv",
    "-e",
    "return execute active tab of front window javascript js",
    "-e",
    "end tell",
    "-e",
    "end run",
]

PROBE_JS = r"""
(() => {
  const round = (value) => Number(value.toFixed(1));
  const isSvgGraphicsElement = (element) =>
    typeof element?.getBBox === "function";
  const toBounds = (element) => {
    const bbox = element.getBBox();
    return {
      x: round(bbox.x),
      y: round(bbox.y),
      width: round(bbox.width),
      height: round(bbox.height),
    };
  };
  const unionBounds = (elements) => {
    const boxes = elements
      .map((element) => element.getBBox())
      .filter((bbox) => bbox.width > 0 || bbox.height > 0);
    if (boxes.length === 0) {
      return null;
    }
    const left = Math.min(...boxes.map((bbox) => bbox.x));
    const top = Math.min(...boxes.map((bbox) => bbox.y));
    const right = Math.max(...boxes.map((bbox) => bbox.x + bbox.width));
    const bottom = Math.max(...boxes.map((bbox) => bbox.y + bbox.height));
    return {
      x: round(left),
      y: round(top),
      width: round(right - left),
      height: round(bottom - top),
    };
  };
  const resolveFontSize = (textElements) => {
    const sizes = textElements
      .map((element) => {
        const attr = Number(element.getAttribute("font-size"));
        if (Number.isFinite(attr) && attr > 0) {
          return attr;
        }
        const computed = Number.parseFloat(window.getComputedStyle(element).fontSize);
        return Number.isFinite(computed) ? computed : 0;
      })
      .filter((value) => value > 0);
    if (sizes.length === 0) {
      return 0;
    }
    return round(Math.min(...sizes));
  };

  const nodes = Array.from(document.querySelectorAll("[data-geometry-node-id]"))
    .flatMap((nodeElement) => {
      const nodeId = nodeElement.getAttribute("data-geometry-node-id");
      if (!nodeId) {
        return [];
      }

      const nodeBoxElement = nodeElement.querySelector('[data-geometry-node-box="1"]');
      if (!nodeBoxElement || !isSvgGraphicsElement(nodeBoxElement)) {
        return [];
      }

      const textElements = Array.from(
        nodeElement.querySelectorAll('[data-geometry-node-text="1"]'),
      ).filter(isSvgGraphicsElement);
      if (textElements.length === 0) {
        return [];
      }

      const textBounds = unionBounds(textElements);
      if (!textBounds) {
        return [];
      }

      const nodeBounds = toBounds(nodeBoxElement);
      const leftPaddingPx = round(textBounds.x - nodeBounds.x);
      const rightPaddingPx = round(
        nodeBounds.x + nodeBounds.width - (textBounds.x + textBounds.width),
      );
      const topPaddingPx = round(textBounds.y - nodeBounds.y);
      const bottomPaddingPx = round(
        nodeBounds.y + nodeBounds.height - (textBounds.y + textBounds.height),
      );

      return [
        {
          nodeId,
          label:
            nodeElement.getAttribute("data-geometry-node-label") ||
            nodeId,
          fontSizePx: resolveFontSize(textElements),
          lineCount: textElements.length,
          topPaddingPx,
          rightPaddingPx,
          bottomPaddingPx,
          leftPaddingPx,
          tightestPaddingPx: round(
            Math.min(topPaddingPx, rightPaddingPx, bottomPaddingPx, leftPaddingPx),
          ),
          textBounds,
          nodeBounds,
        },
      ];
    });

  const params = new URLSearchParams(window.location.search);
  return JSON.stringify({
    sketchId: null,
    stepId: params.get("step"),
    sourceUrl: window.location.href,
    nodes,
  });
})();
""".strip()


def run_edge_probe() -> dict[str, object]:
    raw = subprocess.check_output(
        ["osascript", *EDGE_APPLESCRIPT, "--", PROBE_JS],
        text=True,
    ).strip()
    return json.loads(raw)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Collect real browser-side geometry text metrics from the front Microsoft Edge tab.",
    )
    parser.add_argument("--out", help="Optional file path for the JSON probe payload")
    args = parser.parse_args()

    payload = run_edge_probe()
    formatted = json.dumps(payload, ensure_ascii=False, indent=2)

    if args.out:
      out_path = Path(args.out).expanduser().resolve()
      out_path.parent.mkdir(parents=True, exist_ok=True)
      out_path.write_text(formatted + "\n", encoding="utf-8")

    sys.stdout.write(formatted + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
