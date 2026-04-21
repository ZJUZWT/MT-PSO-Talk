#!/usr/bin/env python3
"""Preflight checks before changing Remotion timeline anchors.

Checks:
1) REMOTION_STEP_SEQUENCE and STEP_FRAME_MAP contain exactly the same step ids.
2) Frame anchors are strictly increasing along sequence.
3) Page10Scene no longer contains hard-coded LOOP_PAGE*_FRAME numeric literals.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


def fail(message: str) -> None:
    print(f"[FAIL] {message}")
    raise SystemExit(1)


def main() -> None:
    repo_root = Path(__file__).resolve().parents[2]
    scene_timeline = repo_root / "SlideApp/src/remotion/sceneTimeline.ts"
    embed_ts = repo_root / "SlideApp/src/remotion/embed.ts"
    page10_scene = repo_root / "SlideApp/src/remotion/pages/Page10Scene.tsx"

    scene_text = scene_timeline.read_text(encoding="utf-8")
    embed_text = embed_ts.read_text(encoding="utf-8")
    page10_text = page10_scene.read_text(encoding="utf-8")

    sequence = re.findall(r'"(page_[0-9]+(?:_[a-z]+)?)"', scene_text)
    frame_pairs = re.findall(
        r"^\s*(page_[0-9]+(?:_[a-z]+)?)\s*:\s*(\d+),?\s*$",
        embed_text,
        flags=re.MULTILINE,
    )
    frame_map = {step: int(frame) for step, frame in frame_pairs}

    if not sequence:
        fail("REMOTION_STEP_SEQUENCE is empty.")
    if not frame_map:
        fail("STEP_FRAME_MAP is empty.")

    sequence_set = set(sequence)
    frame_set = set(frame_map)
    missing_in_map = sorted(sequence_set - frame_set)
    extra_in_map = sorted(frame_set - sequence_set)
    if missing_in_map:
        fail(f"STEP_FRAME_MAP missing steps: {missing_in_map}")
    if extra_in_map:
        fail(f"STEP_FRAME_MAP has extra steps: {extra_in_map}")

    anchor_values = [frame_map[step] for step in sequence]
    if not all(anchor_values[i] < anchor_values[i + 1] for i in range(len(anchor_values) - 1)):
        fail("STEP_FRAME_MAP is not strictly increasing along REMOTION_STEP_SEQUENCE.")

    hardcoded_loop_literals = re.findall(
        r"const\s+LOOP_PAGE[0-9_]+_FRAME\s*=\s*\d+\s*;",
        page10_text,
    )
    if hardcoded_loop_literals:
        fail(
            "Page10Scene still has hard-coded loop frame literals: "
            + ", ".join(hardcoded_loop_literals[:3])
            + (" ..." if len(hardcoded_loop_literals) > 3 else ""),
        )

    print("[PASS] Timeline preflight checks passed.")
    print(f"sequence_count={len(sequence)} map_count={len(frame_map)}")
    print(
        f"first={sequence[0]}:{frame_map[sequence[0]]} "
        f"last={sequence[-1]}:{frame_map[sequence[-1]]}",
    )


if __name__ == "__main__":
    main()

