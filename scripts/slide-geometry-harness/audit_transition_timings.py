#!/usr/bin/env python3
"""Batch-audit all adjacent Remotion step transitions with harness formulas.

This script runs probe_transition_timeline.py for every adjacent pair in
REMOTION_STEP_SEQUENCE and emits:
1) a machine-readable JSON report
2) a human-readable markdown report

The report includes:
- actual transition duration/frame span
- formula-based required duration band
- node/edge action start times and durations
- per-transition suggested frame-span adjustments
- an optional cumulative target frame map (if all spans were normalized)
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import tempfile
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class WorkloadPreset:
    name: str
    hold_before_ms: int
    hold_after_ms: int
    actions: list[dict[str, Any]]


PHASE_ORDER = [
    "hold_before",
    "fade_out",
    "node_motion",
    "edge_motion",
    "fade_in",
    "hold_after",
]


PRESETS: dict[str, WorkloadPreset] = {
    "core_flow": WorkloadPreset(
        name="core_flow",
        hold_before_ms=120,
        hold_after_ms=160,
        actions=[
            {"id": "n_fade_out_old", "kind": "fade_out", "description": "old nodes fade out"},
            {
                "id": "n_move_primary",
                "kind": "node_move",
                "distancePx": 220,
                "description": "primary node translation",
            },
            {
                "id": "e_grow_primary",
                "kind": "edge_grow",
                "lengthPx": 320,
                "description": "primary edge growth",
            },
            {"id": "n_fade_in_new", "kind": "fade_in", "description": "new nodes fade in"},
        ],
    ),
    "flashback_heavy": WorkloadPreset(
        name="flashback_heavy",
        hold_before_ms=140,
        hold_after_ms=200,
        actions=[
            {"id": "n_fade_out_old", "kind": "fade_out", "description": "legacy world fade out"},
            {
                "id": "n_move_main",
                "kind": "node_move",
                "distancePx": 360,
                "description": "main world retreat",
            },
            {
                "id": "n_move_symbol",
                "kind": "node_move",
                "distancePx": 180,
                "description": "symbol/question mark placement",
            },
            {
                "id": "e_grow_main",
                "kind": "edge_grow",
                "lengthPx": 420,
                "description": "main answer edge growth",
            },
            {"id": "n_fade_in_new", "kind": "fade_in", "description": "answer-state fade in"},
        ],
    ),
    "loop_system": WorkloadPreset(
        name="loop_system",
        hold_before_ms=140,
        hold_after_ms=220,
        actions=[
            {"id": "n_fade_out_old", "kind": "fade_out", "description": "legacy nodes fade out"},
            {
                "id": "n_move_primary",
                "kind": "node_move",
                "distancePx": 320,
                "description": "primary topology movement",
            },
            {
                "id": "n_move_secondary",
                "kind": "node_move",
                "distancePx": 180,
                "description": "secondary label/node movement",
            },
            {
                "id": "e_grow_primary",
                "kind": "edge_grow",
                "lengthPx": 460,
                "description": "primary route growth",
            },
            {
                "id": "e_grow_secondary",
                "kind": "edge_grow",
                "lengthPx": 280,
                "description": "secondary route growth",
            },
            {"id": "n_fade_in_new", "kind": "fade_in", "description": "new layer fade in"},
        ],
    ),
    "overlay_page": WorkloadPreset(
        name="overlay_page",
        hold_before_ms=100,
        hold_after_ms=140,
        actions=[
            {"id": "n_fade_out_old", "kind": "fade_out", "description": "base page fade out"},
            {
                "id": "n_move_plate",
                "kind": "node_move",
                "distancePx": 140,
                "description": "overlay plate movement",
            },
            {"id": "n_fade_in_new", "kind": "fade_in", "description": "overlay content fade in"},
        ],
    ),
    "awareness_card": WorkloadPreset(
        name="awareness_card",
        hold_before_ms=100,
        hold_after_ms=140,
        actions=[
            {"id": "n_fade_out_old", "kind": "fade_out", "description": "prior card fade out"},
            {
                "id": "n_move_title",
                "kind": "node_move",
                "distancePx": 110,
                "description": "title row movement",
            },
            {
                "id": "n_move_card",
                "kind": "node_move",
                "distancePx": 140,
                "description": "content card movement",
            },
            {"id": "n_fade_in_new", "kind": "fade_in", "description": "new card fade in"},
        ],
    ),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit all adjacent step transitions with formula timing."
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path.cwd(),
        help="Repository root (default: current working directory).",
    )
    parser.add_argument(
        "--profile",
        choices=["auto", "strict", "wide"],
        default="auto",
        help="Duration profile passed to probe_transition_timeline.py.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("generated/transition-timing-audit"),
        help="Output directory for markdown/json reports.",
    )
    parser.add_argument(
        "--sample-count",
        type=int,
        default=5,
        help="Checkpoint sample count for each probe call.",
    )
    return parser.parse_args()


def load_step_sequence(repo_root: Path) -> list[str]:
    timeline_file = repo_root / "SlideApp/src/remotion/sceneTimeline.ts"
    text = timeline_file.read_text(encoding="utf-8")
    block_match = re.search(
        r"REMOTION_STEP_SEQUENCE:\s*StoryStepId\[\]\s*=\s*\[(.*?)\];",
        text,
        flags=re.S,
    )
    if not block_match:
        raise RuntimeError("Failed to parse REMOTION_STEP_SEQUENCE from sceneTimeline.ts")
    block = block_match.group(1)
    steps = re.findall(r'"(page_[0-9]+(?:_[a-z]+)?)"', block)
    if len(steps) < 2:
        raise RuntimeError("Parsed step sequence is too short.")
    return steps


def step_number(step_id: str) -> int | None:
    match = re.match(r"page_(\d+)", step_id)
    if not match:
        return None
    return int(match.group(1))


def select_preset(from_step: str, to_step: str) -> WorkloadPreset:
    overlay_steps = {"page_04_data", "page_13_img", "page_15_img", "page_18_img"}
    if from_step in overlay_steps or to_step in overlay_steps:
        return PRESETS["overlay_page"]
    if from_step == "page_09" and to_step == "page_10":
        return PRESETS["flashback_heavy"]

    from_num = step_number(from_step)
    to_num = step_number(to_step)
    if from_num is not None and to_num is not None:
        if from_num >= 21 or to_num >= 21:
            return PRESETS["awareness_card"]
        if 11 <= from_num <= 20 and 11 <= to_num <= 20:
            return PRESETS["loop_system"]

    return PRESETS["core_flow"]


def build_workload_payload(preset: WorkloadPreset) -> dict[str, Any]:
    return {
        "holdBeforeMs": preset.hold_before_ms,
        "holdAfterMs": preset.hold_after_ms,
        "phaseOrder": PHASE_ORDER,
        "actions": preset.actions,
    }


def run_probe(
    repo_root: Path,
    from_step: str,
    to_step: str,
    profile: str,
    sample_count: int,
    workload_payload: dict[str, Any],
) -> dict[str, Any]:
    probe_script = repo_root / "scripts/slide-geometry-harness/probe_transition_timeline.py"
    with tempfile.NamedTemporaryFile("w", suffix=".json", encoding="utf-8", delete=False) as fp:
        json.dump(workload_payload, fp, ensure_ascii=False, indent=2)
        workload_path = Path(fp.name)

    try:
        cmd = [
            "python3",
            str(probe_script),
            "--repo-root",
            str(repo_root),
            "--from-step",
            from_step,
            "--to-step",
            to_step,
            "--profile",
            profile,
            "--sample-count",
            str(sample_count),
            "--workload-json",
            str(workload_path),
        ]
        completed = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=True,
        )
        return json.loads(completed.stdout)
    finally:
        try:
            workload_path.unlink(missing_ok=True)
        except OSError:
            pass


def format_action_line(action: dict[str, Any]) -> str:
    action_id = action.get("id", "-")
    kind = action.get("kind", "-")
    start_ms = float(action.get("startMs", 0.0))
    end_ms = float(action.get("endMs", 0.0))
    duration_ms = float(action.get("durationMs", 0.0))
    return (
        f"- `{action_id}` ({kind}): start={start_ms:.0f}ms, "
        f"end={end_ms:.0f}ms, duration={duration_ms:.0f}ms"
    )


def collect_transition_rows(
    sequence: list[str],
    probes: list[dict[str, Any]],
    presets: list[str],
) -> tuple[list[dict[str, Any]], dict[str, int]]:
    rows: list[dict[str, Any]] = []
    target_step_frames: dict[str, int] = {sequence[0]: int(probes[0]["fromFrame"])}

    rolling_frame = target_step_frames[sequence[0]]
    for i, probe in enumerate(probes):
        eval_obj = probe.get("timingStandardEval") or {}
        plan_obj = probe.get("timingStandardPlan") or {}
        actions = plan_obj.get("actions") or []

        node_starts = [
            float(action.get("startMs", 0.0))
            for action in actions
            if action.get("category") == "node"
        ]
        edge_starts = [
            float(action.get("startMs", 0.0))
            for action in actions
            if action.get("category") == "edge"
        ]

        suggested_span = int(
            eval_obj.get(
                "suggestedFrameSpan",
                probe.get("frameSpan", 0),
            )
        )
        rolling_frame += suggested_span
        target_step_frames[probe["toStepId"]] = rolling_frame

        rows.append(
            {
                "fromStepId": probe["fromStepId"],
                "toStepId": probe["toStepId"],
                "preset": presets[i],
                "frameSpan": int(probe.get("frameSpan", 0)),
                "durationSec": float(probe.get("durationSec", 0.0)),
                "requiredSec": float(eval_obj.get("requiredSec", 0.0)),
                "allowedMinSec": float(eval_obj.get("allowedMinSec", 0.0)),
                "allowedMaxSec": float(eval_obj.get("allowedMaxSec", 0.0)),
                "verdict": str(eval_obj.get("verdict", "unknown")),
                "suggestedFrameSpan": suggested_span,
                "suggestedDeltaFrames": int(eval_obj.get("suggestedDeltaFrames", 0)),
                "nodeStartMs": min(node_starts) if node_starts else None,
                "edgeStartMs": min(edge_starts) if edge_starts else None,
                "actions": actions,
            }
        )
    return rows, target_step_frames


def build_markdown(
    sequence: list[str],
    rows: list[dict[str, Any]],
    target_step_frames: dict[str, int],
) -> str:
    too_short = sum(1 for row in rows if row["verdict"] == "too_short")
    too_long = sum(1 for row in rows if row["verdict"] == "too_long")
    in_range = sum(1 for row in rows if row["verdict"] == "in_range")

    lines: list[str] = []
    lines.append("# Transition Timing Audit")
    lines.append("")
    lines.append("- Formula source: `Harness Registry v1.0`")
    lines.append(
        "- Policy: `node_move / edge_grow / fade + hold_before -> fade_out -> node_motion -> edge_motion -> fade_in -> hold_after`"
    )
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append(f"- Transition count: `{len(rows)}`")
    lines.append(f"- `in_range`: `{in_range}`")
    lines.append(f"- `too_short`: `{too_short}`")
    lines.append(f"- `too_long`: `{too_long}`")
    lines.append("")
    lines.append("## Transition Table")
    lines.append("")
    lines.append(
        "| Transition | Preset | Actual(s) | Required(s) | Allowed(s) | Verdict | NodeStart(ms) | EdgeStart(ms) | CurrentFrames | SuggestedFrames | ΔFrames |"
    )
    lines.append(
        "|---|---|---:|---:|---:|---|---:|---:|---:|---:|---:|"
    )
    for row in rows:
        node_start = "-" if row["nodeStartMs"] is None else f"{row['nodeStartMs']:.0f}"
        edge_start = "-" if row["edgeStartMs"] is None else f"{row['edgeStartMs']:.0f}"
        lines.append(
            "| "
            f"`{row['fromStepId']} -> {row['toStepId']}` | "
            f"`{row['preset']}` | "
            f"{row['durationSec']:.3f} | "
            f"{row['requiredSec']:.3f} | "
            f"{row['allowedMinSec']:.3f} ~ {row['allowedMaxSec']:.3f} | "
            f"`{row['verdict']}` | "
            f"{node_start} | {edge_start} | "
            f"{row['frameSpan']} | {row['suggestedFrameSpan']} | "
            f"{row['suggestedDeltaFrames']} |"
        )

    lines.append("")
    lines.append("## Per-Transition Action Start Times")
    lines.append("")
    for row in rows:
        lines.append(f"### `{row['fromStepId']} -> {row['toStepId']}` (`{row['preset']}`)")
        lines.append("")
        if not row["actions"]:
            lines.append("- No actions resolved.")
        else:
            for action in row["actions"]:
                lines.append(format_action_line(action))
        lines.append("")

    lines.append("## Suggested Step Frames (If Applying SuggestedFrameSpan)")
    lines.append("")
    lines.append("| Step | SuggestedFrame |")
    lines.append("|---|---:|")
    for step in sequence:
        frame = target_step_frames.get(step)
        if frame is None:
            continue
        lines.append(f"| `{step}` | {frame} |")
    lines.append("")
    return "\n".join(lines)


def main() -> None:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    output_dir = (repo_root / args.output_dir).resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    sequence = load_step_sequence(repo_root)

    probes: list[dict[str, Any]] = []
    presets_used: list[str] = []
    for idx in range(len(sequence) - 1):
        from_step = sequence[idx]
        to_step = sequence[idx + 1]
        preset = select_preset(from_step, to_step)
        workload = build_workload_payload(preset)
        probe = run_probe(
            repo_root=repo_root,
            from_step=from_step,
            to_step=to_step,
            profile=args.profile,
            sample_count=args.sample_count,
            workload_payload=workload,
        )
        probes.append(probe)
        presets_used.append(preset.name)

    rows, target_step_frames = collect_transition_rows(sequence, probes, presets_used)

    payload = {
        "generatedAt": datetime.now().isoformat(timespec="seconds"),
        "profile": args.profile,
        "sequence": sequence,
        "transitions": rows,
        "suggestedStepFrames": target_step_frames,
    }

    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    json_path = output_dir / f"transition-timing-audit-{stamp}.json"
    md_path = output_dir / f"transition-timing-audit-{stamp}.md"
    json_path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8")
    md_path.write_text(build_markdown(sequence, rows, target_step_frames), encoding="utf-8")

    print(json.dumps({"json": str(json_path), "markdown": str(md_path)}, ensure_ascii=False))


if __name__ == "__main__":
    main()
