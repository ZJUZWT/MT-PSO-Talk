#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import math
import re
import sys
from pathlib import Path
from typing import Any


DEFAULT_FPS = 60
DEFAULT_SAMPLE_COUNT = 5

TIMING_STANDARD_VERSION = "v1.0"
DEFAULT_HOLD_BEFORE_MS = 120.0
DEFAULT_HOLD_AFTER_MS = 180.0
STANDARD_DURATION_MAX_RATIO = 1.35
STANDARD_DURATION_TARGET_RATIO = 1.2
DEFAULT_PHASE_ORDER = [
    "hold_before",
    "fade_out",
    "node_motion",
    "edge_motion",
    "fade_in",
    "hold_after",
]

# Formula policy:
# duration_ms = clamp(base_ms + metric_value * per_unit_ms, min_ms, max_ms)
TIMING_KIND_POLICY: dict[str, dict[str, Any]] = {
    "node_move": {
        "category": "node",
        "metric_key": "distancePx",
        "base_ms": 180.0,
        "per_unit_ms": 0.95,
        "min_ms": 180.0,
        "max_ms": 960.0,
    },
    "edge_grow": {
        "category": "edge",
        "metric_key": "lengthPx",
        "base_ms": 140.0,
        "per_unit_ms": 0.75,
        "min_ms": 140.0,
        "max_ms": 840.0,
    },
    "fade_in": {
        "category": "node",
        "metric_key": None,
        "base_ms": 220.0,
        "per_unit_ms": 0.0,
        "min_ms": 180.0,
        "max_ms": 420.0,
    },
    "fade_out": {
        "category": "node",
        "metric_key": None,
        "base_ms": 220.0,
        "per_unit_ms": 0.0,
        "min_ms": 180.0,
        "max_ms": 420.0,
    },
}

ACTION_KIND_ALIASES = {
    "node_translate": "node_move",
    "node_translation": "node_move",
    "translate": "node_move",
    "edge_extend": "edge_grow",
    "edge_growth": "edge_grow",
    "edge_expand": "edge_grow",
    "fadein": "fade_in",
    "fadeout": "fade_out",
}


def resolve_repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_step_frame_map(embed_ts_path: Path) -> dict[str, int]:
    text = read_text(embed_ts_path)
    match = re.search(
        r"STEP_FRAME_MAP:\s*Record<StoryStepId,\s*number>\s*=\s*\{(.*?)\};",
        text,
        re.DOTALL,
    )
    if not match:
        raise RuntimeError(f"Could not parse STEP_FRAME_MAP from {embed_ts_path}")

    body = match.group(1)
    pairs = re.findall(r"^\s*([a-zA-Z0-9_]+)\s*:\s*(\d+)\s*,?\s*$", body, re.MULTILINE)
    if not pairs:
        raise RuntimeError(f"STEP_FRAME_MAP in {embed_ts_path} appears empty")

    return {step_id: int(frame) for step_id, frame in pairs}


def parse_step_sequence(scene_timeline_ts_path: Path) -> list[str]:
    text = read_text(scene_timeline_ts_path)
    match = re.search(
        r"REMOTION_STEP_SEQUENCE:\s*StoryStepId\[\]\s*=\s*\[(.*?)\];",
        text,
        re.DOTALL,
    )
    if not match:
        raise RuntimeError(
            f"Could not parse REMOTION_STEP_SEQUENCE from {scene_timeline_ts_path}",
        )

    body = match.group(1)
    step_ids = re.findall(r'"([a-zA-Z0-9_]+)"', body)
    if not step_ids:
        raise RuntimeError(
            f"REMOTION_STEP_SEQUENCE in {scene_timeline_ts_path} appears empty",
        )
    return step_ids


def infer_to_step(from_step: str, sequence: list[str]) -> str:
    if from_step not in sequence:
        raise RuntimeError(f"Step '{from_step}' not found in REMOTION_STEP_SEQUENCE")

    index = sequence.index(from_step)
    if index >= len(sequence) - 1:
        raise RuntimeError(f"Step '{from_step}' is the last step and has no next transition")

    return sequence[index + 1]


def resolve_duration_band(profile: str, from_step: str, to_step: str) -> tuple[float, float, str]:
    if profile == "strict":
        return 1.2, 2.2, "strict"

    if profile == "wide":
        return 1.0, 2.8, "wide"

    # auto profile
    if "_img" in from_step or "_img" in to_step or "_data" in from_step or "_data" in to_step:
        return 0.9, 1.8, "auto-insert"

    return 1.2, 2.6, "auto-default"


def clamp(value: float, min_value: float, max_value: float) -> float:
    return max(min_value, min(max_value, value))


def as_float(value: Any) -> float | None:
    if value is None:
        return None
    if isinstance(value, bool):
        return None
    if isinstance(value, (int, float)):
        return float(value)
    if isinstance(value, str):
        try:
            return float(value.strip())
        except ValueError:
            return None
    return None


def normalize_action_kind(kind_raw: str) -> str:
    normalized = kind_raw.strip().lower()
    return ACTION_KIND_ALIASES.get(normalized, normalized)


def default_phase_for_kind(kind: str) -> str:
    if kind == "fade_out":
        return "fade_out"
    if kind == "node_move":
        return "node_motion"
    if kind == "edge_grow":
        return "edge_motion"
    if kind == "fade_in":
        return "fade_in"
    return "node_motion"


def resolve_metric_value(action: dict[str, Any], metric_key: str | None) -> float:
    if metric_key is None:
        return 0.0

    if metric_key in action:
        return as_float(action.get(metric_key)) or 0.0

    alias_map = {
        "distancePx": ["distance", "distance_px", "distancePX", "lengthPx"],
        "lengthPx": ["length", "length_px", "lengthPX", "distancePx"],
    }
    for alias in alias_map.get(metric_key, []):
        if alias in action:
            return as_float(action.get(alias)) or 0.0

    return 0.0


def load_workload(workload_json_path: Path | None) -> dict[str, Any] | None:
    if not workload_json_path:
        return None

    payload = json.loads(workload_json_path.read_text(encoding="utf-8"))
    if isinstance(payload, list):
        payload = {"actions": payload}
    if not isinstance(payload, dict):
        raise RuntimeError("workload json must be an object or an action list")

    actions = payload.get("actions", [])
    if not isinstance(actions, list):
        raise RuntimeError("workload json field 'actions' must be a list")

    phase_order_raw = payload.get("phaseOrder", DEFAULT_PHASE_ORDER)
    if not isinstance(phase_order_raw, list):
        raise RuntimeError("workload json field 'phaseOrder' must be a list when provided")
    phase_order = [str(item).strip() for item in phase_order_raw if str(item).strip()]
    if not phase_order:
        phase_order = DEFAULT_PHASE_ORDER[:]

    hold_before_ms = as_float(payload.get("holdBeforeMs"))
    hold_after_ms = as_float(payload.get("holdAfterMs"))

    return {
        "actions": actions,
        "phaseOrder": phase_order,
        "holdBeforeMs": (
            hold_before_ms if hold_before_ms is not None else DEFAULT_HOLD_BEFORE_MS
        ),
        "holdAfterMs": (
            hold_after_ms if hold_after_ms is not None else DEFAULT_HOLD_AFTER_MS
        ),
    }


def build_timing_standard_plan(
    workload: dict[str, Any] | None,
) -> tuple[dict[str, Any] | None, list[str]]:
    if not workload:
        return None, []

    warnings: list[str] = []
    action_rows: list[dict[str, Any]] = []
    actions_raw = workload.get("actions", [])
    phase_order = list(workload.get("phaseOrder", DEFAULT_PHASE_ORDER))
    hold_before_ms = max(0.0, as_float(workload.get("holdBeforeMs")) or DEFAULT_HOLD_BEFORE_MS)
    hold_after_ms = max(0.0, as_float(workload.get("holdAfterMs")) or DEFAULT_HOLD_AFTER_MS)

    for index, raw in enumerate(actions_raw):
        if not isinstance(raw, dict):
            warnings.append(f"actions[{index}] ignored: not an object")
            continue

        action_id = str(raw.get("id") or f"action_{index + 1}").strip()
        kind_raw = str(raw.get("kind") or "").strip()
        kind = normalize_action_kind(kind_raw)
        policy = TIMING_KIND_POLICY.get(kind)
        if policy is None:
            warnings.append(f"{action_id} ignored: unknown kind '{kind_raw}'")
            continue

        phase = str(raw.get("phase") or default_phase_for_kind(kind)).strip()
        if not phase:
            phase = default_phase_for_kind(kind)

        manual_duration = as_float(raw.get("durationMs"))
        metric_key: str | None = policy["metric_key"]
        metric_value = resolve_metric_value(raw, metric_key)
        metric_key_used = metric_key or "-"
        if metric_key and metric_value <= 0.0 and manual_duration is None:
            warnings.append(
                f"{action_id} kind={kind}: {metric_key} missing/<=0, fallback to base duration",
            )

        base_ms = float(policy["base_ms"])
        per_unit_ms = float(policy["per_unit_ms"])
        min_ms = float(policy["min_ms"])
        max_ms = float(policy["max_ms"])
        formula_duration = clamp(base_ms + metric_value * per_unit_ms, min_ms, max_ms)
        resolved_duration = (
            clamp(manual_duration, min_ms, max_ms)
            if manual_duration is not None
            else formula_duration
        )

        action_rows.append(
            {
                "id": action_id,
                "kind": kind,
                "phase": phase,
                "category": str(policy["category"]),
                "metricKey": metric_key_used,
                "metricValue": round(metric_value, 2),
                "durationMs": round(resolved_duration, 1),
                "formula": (
                    f"clamp({base_ms:.0f} + {metric_value:.1f}*{per_unit_ms:.2f}, "
                    f"{min_ms:.0f}, {max_ms:.0f})"
                    if metric_key
                    else f"fixed={base_ms:.0f}ms clamp({min_ms:.0f}, {max_ms:.0f})"
                ),
                "source": "manual" if manual_duration is not None else "formula",
                "description": str(raw.get("description") or "").strip(),
            },
        )

    grouped: dict[str, list[dict[str, Any]]] = {}
    for action in action_rows:
        grouped.setdefault(action["phase"], []).append(action)

    resolved_phase_order = phase_order[:]
    for phase in grouped:
        if phase not in resolved_phase_order:
            resolved_phase_order.append(phase)

    phase_rows: list[dict[str, Any]] = []
    cursor_ms = 0.0
    for phase in resolved_phase_order:
        actions = grouped.get(phase, [])
        action_max = max((float(item["durationMs"]) for item in actions), default=0.0)
        if phase == "hold_before":
            phase_duration = max(hold_before_ms, action_max)
        elif phase == "hold_after":
            phase_duration = max(hold_after_ms, action_max)
        else:
            phase_duration = action_max

        if phase_duration <= 0 and not actions:
            continue

        phase_start = cursor_ms
        phase_end = cursor_ms + phase_duration
        for item in actions:
            item["startMs"] = round(phase_start, 1)
            item["endMs"] = round(phase_start + float(item["durationMs"]), 1)

        phase_rows.append(
            {
                "id": phase,
                "startMs": round(phase_start, 1),
                "endMs": round(phase_end, 1),
                "durationMs": round(phase_duration, 1),
                "actionIds": [item["id"] for item in actions],
            },
        )
        cursor_ms = phase_end

    required_ms = round(cursor_ms, 1)
    required_sec = required_ms / 1000.0
    allowed_max_sec = required_sec * STANDARD_DURATION_MAX_RATIO
    plan = {
        "version": TIMING_STANDARD_VERSION,
        "phaseOrder": resolved_phase_order,
        "holdBeforeMs": round(hold_before_ms, 1),
        "holdAfterMs": round(hold_after_ms, 1),
        "phases": phase_rows,
        "actions": action_rows,
        "requiredDurationSec": round(required_sec, 3),
        "allowedMinSec": round(required_sec, 3),
        "allowedMaxSec": round(allowed_max_sec, 3),
        "formulaPolicy": {
            "node_move": "duration_ms = clamp(180 + distancePx*0.95, 180, 960)",
            "edge_grow": "duration_ms = clamp(140 + lengthPx*0.75, 140, 840)",
            "fade_in": "duration_ms = 220 (clamp 180~420)",
            "fade_out": "duration_ms = 220 (clamp 180~420)",
        },
    }
    return plan, warnings


def evaluate_timing_standard(
    duration_sec: float,
    frame_span: int,
    fps: int,
    timing_plan: dict[str, Any] | None,
) -> dict[str, Any] | None:
    if not timing_plan:
        return None

    required_sec = float(timing_plan["requiredDurationSec"])
    allowed_min_sec = float(timing_plan["allowedMinSec"])
    allowed_max_sec = float(timing_plan["allowedMaxSec"])
    if duration_sec < allowed_min_sec:
        verdict = "too_short"
        target_sec = required_sec
    elif duration_sec > allowed_max_sec:
        verdict = "too_long"
        target_sec = required_sec * STANDARD_DURATION_TARGET_RATIO
    else:
        verdict = "in_range"
        target_sec = duration_sec

    if verdict == "too_short":
        # Guard against rounding down below the required minimum.
        suggested_frame_span = math.ceil(target_sec * fps - 1e-9)
        if suggested_frame_span <= frame_span:
            suggested_frame_span = frame_span + 1
    elif verdict == "too_long":
        # Keep enough room but avoid over-stretching when shortening.
        suggested_frame_span = math.floor(target_sec * fps + 1e-9)
        if suggested_frame_span >= frame_span:
            suggested_frame_span = max(1, frame_span - 1)
    else:
        suggested_frame_span = frame_span

    return {
        "version": TIMING_STANDARD_VERSION,
        "verdict": verdict,
        "requiredSec": round(required_sec, 3),
        "allowedMinSec": round(allowed_min_sec, 3),
        "allowedMaxSec": round(allowed_max_sec, 3),
        "suggestedFrameSpan": suggested_frame_span,
        "suggestedDeltaFrames": suggested_frame_span - frame_span,
    }


def load_events(events_json_path: Path | None) -> dict[str, dict[str, str]]:
    if not events_json_path:
        return {}

    payload = json.loads(events_json_path.read_text(encoding="utf-8"))
    # Supported formats:
    # 1) {"checkpoints":[{"progress":0.25,"node_action":"...","edge_action":"..."}]}
    # 2) {"by_label":{"Q1":{"node_action":"...","edge_action":"..."}}}
    # 3) {"by_progress":{"0.25":{"node_action":"...","edge_action":"..."}}}
    result: dict[str, dict[str, str]] = {}

    if isinstance(payload, dict):
        checkpoints = payload.get("checkpoints")
        if isinstance(checkpoints, list):
            for entry in checkpoints:
                if not isinstance(entry, dict):
                    continue
                progress = entry.get("progress")
                if isinstance(progress, (int, float)):
                    key = f"p:{float(progress):.4f}"
                    result[key] = {
                        "node_action": str(entry.get("node_action", "")).strip(),
                        "edge_action": str(entry.get("edge_action", "")).strip(),
                    }

        by_label = payload.get("by_label")
        if isinstance(by_label, dict):
            for label, entry in by_label.items():
                if not isinstance(entry, dict):
                    continue
                result[f"l:{label}"] = {
                    "node_action": str(entry.get("node_action", "")).strip(),
                    "edge_action": str(entry.get("edge_action", "")).strip(),
                }

        by_progress = payload.get("by_progress")
        if isinstance(by_progress, dict):
            for progress_raw, entry in by_progress.items():
                if not isinstance(entry, dict):
                    continue
                try:
                    progress = float(progress_raw)
                except ValueError:
                    continue
                result[f"p:{progress:.4f}"] = {
                    "node_action": str(entry.get("node_action", "")).strip(),
                    "edge_action": str(entry.get("edge_action", "")).strip(),
                }

    return result


def resolve_checkpoint_labels(sample_count: int) -> list[str]:
    if sample_count == 5:
        return ["Start", "Q1", "Mid", "Q3", "End"]
    if sample_count == 3:
        return ["Start", "Mid", "End"]
    return [f"P{i + 1}" for i in range(sample_count)]


def resolve_duration_verdict(
    duration_sec: float,
    min_sec: float,
    max_sec: float,
) -> str:
    if duration_sec < min_sec:
        return "too_short"
    if duration_sec > max_sec:
        return "too_long"
    return "in_range"


def summarize_checkpoint_by_plan(
    elapsed_sec: float,
    timing_plan: dict[str, Any] | None,
) -> tuple[str, str]:
    if timing_plan is None:
        return "未提供具体节点动作", "未提供具体边动作"

    elapsed_ms = elapsed_sec * 1000.0
    phases = timing_plan.get("phases", [])
    actions = timing_plan.get("actions", [])

    current_phase_id = "unknown"
    for phase in phases:
        start_ms = float(phase["startMs"])
        end_ms = float(phase["endMs"])
        if start_ms <= elapsed_ms <= end_ms + 0.1:
            current_phase_id = str(phase["id"])
            break

    node_items: list[str] = []
    edge_items: list[str] = []
    for action in actions:
        start_ms = float(action.get("startMs", 0.0))
        end_ms = float(action.get("endMs", 0.0))
        if not (start_ms <= elapsed_ms <= end_ms + 0.1):
            continue
        entry = f"{action['id']}({action['kind']})"
        if action.get("category") == "edge":
            edge_items.append(entry)
        else:
            node_items.append(entry)

    if not node_items:
        if current_phase_id in {"fade_in", "fade_out"}:
            node_desc = f"{current_phase_id} 阶段执行"
        elif current_phase_id == "node_motion":
            node_desc = "node_motion 阶段执行"
        elif current_phase_id in {"hold_before", "hold_after"}:
            node_desc = f"{current_phase_id} 静置"
        else:
            node_desc = "无显著节点变化"
    else:
        node_desc = "、".join(node_items)

    if not edge_items:
        if current_phase_id == "edge_motion":
            edge_desc = "edge_motion 阶段执行"
        elif current_phase_id in {"hold_before", "hold_after"}:
            edge_desc = f"{current_phase_id} 静置"
        else:
            edge_desc = "无显著边变化"
    else:
        edge_desc = "、".join(edge_items)

    return node_desc, edge_desc


def build_checkpoints(
    from_frame: int,
    to_frame: int,
    fps: int,
    sample_count: int,
    events: dict[str, dict[str, str]],
    timing_plan: dict[str, Any] | None,
) -> list[dict[str, Any]]:
    frame_span = to_frame - from_frame
    labels = resolve_checkpoint_labels(sample_count)
    checkpoints: list[dict[str, Any]] = []

    for index in range(sample_count):
        progress = 0.0 if sample_count == 1 else index / (sample_count - 1)
        frame = round(from_frame + progress * frame_span)
        elapsed_sec = (frame - from_frame) / fps
        label = labels[index]
        progress_key = f"p:{progress:.4f}"
        label_key = f"l:{label}"
        event = events.get(progress_key) or events.get(label_key) or {}
        default_node_action, default_edge_action = summarize_checkpoint_by_plan(
            elapsed_sec=elapsed_sec,
            timing_plan=timing_plan,
        )

        checkpoints.append(
            {
                "index": index + 1,
                "label": label,
                "progress": round(progress, 4),
                "frame": frame,
                "elapsed_sec": round(elapsed_sec, 3),
                "node_action": event.get("node_action", "").strip() or default_node_action,
                "edge_action": event.get("edge_action", "").strip() or default_edge_action,
            },
        )

    return checkpoints


def build_markdown(
    from_step: str,
    to_step: str,
    fps: int,
    from_frame: int,
    to_frame: int,
    duration_sec: float,
    duration_verdict: str,
    min_sec: float,
    max_sec: float,
    suggested_span_frames: int,
    suggested_delta_frames: int,
    preserve_base_result: str | None,
    checkpoints: list[dict[str, Any]],
    timing_plan: dict[str, Any] | None,
    timing_standard_eval: dict[str, Any] | None,
    warnings: list[str],
) -> str:
    lines: list[str] = []
    lines.append(f"## 过渡动画剧本（{from_step} -> {to_step}）")
    lines.append("")
    lines.append(f"- 帧窗口：`{from_frame} -> {to_frame}`（`{to_frame - from_frame}` frames @ {fps}fps）")
    lines.append(f"- 总时长：`{duration_sec:.3f}s`")
    lines.append(
        f"- 时长评估：`{duration_verdict}`（目标区间 `{min_sec:.2f}s ~ {max_sec:.2f}s`）",
    )
    lines.append(
        f"- 调整建议：目标帧长 `{suggested_span_frames}`，相对当前 `Δ{suggested_delta_frames:+d}` frames",
    )
    if preserve_base_result:
        lines.append(f"- 原始动画保护检查：`{preserve_base_result}`")
    if timing_standard_eval:
        lines.append(
            "- 标准时序评估："
            f"`{timing_standard_eval['verdict']}`"
            f"（标准下限 `{timing_standard_eval['allowedMinSec']:.3f}s`，"
            f"上限 `{timing_standard_eval['allowedMaxSec']:.3f}s`）",
        )
        lines.append(
            "- 标准调整建议："
            f"目标帧长 `{timing_standard_eval['suggestedFrameSpan']}`，"
            f"相对当前 `Δ{timing_standard_eval['suggestedDeltaFrames']:+d}` frames",
        )
    if warnings:
        lines.append(f"- workload 警告：`{len(warnings)}` 条（详见 JSON `timingStandardWarnings`）")
    lines.append("")

    if timing_plan:
        lines.append(f"### Harness 动画标准（{timing_plan['version']}）")
        lines.append("")
        lines.append(
            f"- 标准相位顺序：`{' -> '.join(timing_plan['phaseOrder'])}`",
        )
        lines.append(
            f"- 标准推荐最短时长：`{timing_plan['requiredDurationSec']:.3f}s`",
        )
        lines.append(
            f"- 标准允许上限：`{timing_plan['allowedMaxSec']:.3f}s`",
        )
        lines.append("")
        lines.append("| Phase | 开始(ms) | 结束(ms) | 时长(ms) | 动作 |")
        lines.append("|---|---:|---:|---:|---|")
        for phase in timing_plan["phases"]:
            lines.append(
                f"| {phase['id']} | {phase['startMs']:.1f} | {phase['endMs']:.1f} "
                f"| {phase['durationMs']:.1f} | {', '.join(phase['actionIds']) or '-'} |",
            )
        lines.append("")
        lines.append("| 动作 id | 类型 | 相位 | 指标 | 计算时长(ms) | 来源 |")
        lines.append("|---|---|---|---|---:|---|")
        for action in timing_plan["actions"]:
            metric_desc = (
                f"{action['metricKey']}={action['metricValue']}"
                if action["metricKey"] != "-"
                else "-"
            )
            lines.append(
                f"| {action['id']} | {action['kind']} | {action['phase']} "
                f"| {metric_desc} | {action['durationMs']:.1f} | {action['source']} |",
            )
        lines.append("")

    lines.append("| 时间点 | 帧 | 相对时间(s) | Node 行为 | Edge 行为 |")
    lines.append("|---|---:|---:|---|---|")
    for checkpoint in checkpoints:
        lines.append(
            "| "
            f"{checkpoint['label']} "
            f"| {checkpoint['frame']} "
            f"| {checkpoint['elapsed_sec']:.3f} "
            f"| {checkpoint['node_action']} "
            f"| {checkpoint['edge_action']} |",
        )
    lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Extract transition duration/checkpoints from SlideApp step frames and emit "
            "JSON + optional markdown script block."
        ),
    )
    parser.add_argument("--repo-root", type=Path, help="Repo root path")
    parser.add_argument("--from-step", required=True, help="From step id, e.g. page_14")
    parser.add_argument("--to-step", help="To step id; omitted means next step from sequence")
    parser.add_argument(
        "--profile",
        choices=["auto", "strict", "wide"],
        default="auto",
        help="Duration profile band for short/long evaluation",
    )
    parser.add_argument(
        "--sample-count",
        type=int,
        default=DEFAULT_SAMPLE_COUNT,
        help="Number of checkpoints (>=2 recommended)",
    )
    parser.add_argument(
        "--events-json",
        type=Path,
        help="Optional JSON to pre-fill node/edge actions at checkpoints",
    )
    parser.add_argument(
        "--workload-json",
        type=Path,
        help=(
            "Optional action workload JSON for formula-based timing standard. "
            "Format: {actions:[{id,kind,distancePx|lengthPx,phase?,durationMs?}], "
            "holdBeforeMs?, holdAfterMs?, phaseOrder?}"
        ),
    )
    parser.add_argument(
        "--emit-markdown",
        action="store_true",
        help="Emit markdown section in addition to JSON",
    )
    parser.add_argument(
        "--base-seconds",
        type=float,
        help="Original base animation duration before insertion",
    )
    parser.add_argument(
        "--insert-seconds",
        type=float,
        help="Added pre/mid animation duration",
    )
    args = parser.parse_args()

    if args.sample_count < 2:
        raise SystemExit("--sample-count must be >= 2")

    repo_root = args.repo_root.resolve() if args.repo_root else resolve_repo_root()
    embed_ts_path = repo_root / "SlideApp" / "src" / "remotion" / "embed.ts"
    scene_timeline_path = repo_root / "SlideApp" / "src" / "remotion" / "sceneTimeline.ts"
    if not embed_ts_path.exists():
        raise SystemExit(f"Missing embed.ts: {embed_ts_path}")
    if not scene_timeline_path.exists():
        raise SystemExit(f"Missing sceneTimeline.ts: {scene_timeline_path}")

    step_frame_map = parse_step_frame_map(embed_ts_path)
    sequence = parse_step_sequence(scene_timeline_path)

    from_step = args.from_step
    to_step = args.to_step or infer_to_step(from_step, sequence)
    if from_step not in step_frame_map:
        raise SystemExit(f"Unknown from-step: {from_step}")
    if to_step not in step_frame_map:
        raise SystemExit(f"Unknown to-step: {to_step}")

    from_frame = step_frame_map[from_step]
    to_frame = step_frame_map[to_step]
    if to_frame <= from_frame:
        raise SystemExit(
            f"Invalid frame order: {from_step}={from_frame}, {to_step}={to_frame}",
        )

    frame_span = to_frame - from_frame
    duration_sec = frame_span / DEFAULT_FPS
    min_sec, max_sec, resolved_profile = resolve_duration_band(args.profile, from_step, to_step)
    duration_verdict = resolve_duration_verdict(duration_sec, min_sec, max_sec)
    target_duration_sec = min(max(duration_sec, min_sec), max_sec)
    suggested_span_frames = round(target_duration_sec * DEFAULT_FPS)
    suggested_delta_frames = suggested_span_frames - frame_span

    preserve_base_result: str | None = None
    if args.base_seconds is not None and args.insert_seconds is not None:
        required = args.base_seconds + args.insert_seconds
        preserve_base_result = (
            "pass"
            if duration_sec + 1e-9 >= required
            else f"fail (current {duration_sec:.3f}s < required {required:.3f}s)"
        )

    events = load_events(args.events_json.resolve() if args.events_json else None)
    workload = load_workload(args.workload_json.resolve() if args.workload_json else None)
    timing_plan, timing_warnings = build_timing_standard_plan(workload)
    timing_standard_eval = evaluate_timing_standard(
        duration_sec=duration_sec,
        frame_span=frame_span,
        fps=DEFAULT_FPS,
        timing_plan=timing_plan,
    )
    checkpoints = build_checkpoints(
        from_frame=from_frame,
        to_frame=to_frame,
        fps=DEFAULT_FPS,
        sample_count=args.sample_count,
        events=events,
        timing_plan=timing_plan,
    )

    payload: dict[str, Any] = {
        "fromStepId": from_step,
        "toStepId": to_step,
        "fromFrame": from_frame,
        "toFrame": to_frame,
        "frameSpan": frame_span,
        "fps": DEFAULT_FPS,
        "durationSec": round(duration_sec, 3),
        "durationBandSec": {
            "profile": resolved_profile,
            "min": min_sec,
            "max": max_sec,
        },
        "durationVerdict": duration_verdict,
        "suggestedFrameSpan": suggested_span_frames,
        "suggestedDeltaFrames": suggested_delta_frames,
        "preserveBaseDurationCheck": preserve_base_result,
        "timingStandardPlan": timing_plan,
        "timingStandardEval": timing_standard_eval,
        "timingStandardWarnings": timing_warnings,
        "checkpoints": checkpoints,
    }

    if args.emit_markdown:
        payload["markdown"] = build_markdown(
            from_step=from_step,
            to_step=to_step,
            fps=DEFAULT_FPS,
            from_frame=from_frame,
            to_frame=to_frame,
            duration_sec=duration_sec,
            duration_verdict=duration_verdict,
            min_sec=min_sec,
            max_sec=max_sec,
            suggested_span_frames=suggested_span_frames,
            suggested_delta_frames=suggested_delta_frames,
            preserve_base_result=preserve_base_result,
            checkpoints=checkpoints,
            timing_plan=timing_plan,
            timing_standard_eval=timing_standard_eval,
            warnings=timing_warnings,
        )

    json.dump(payload, sys.stdout, ensure_ascii=False, indent=2)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
