#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass, field
from datetime import datetime, timezone
from pathlib import Path


STATE_FILENAME = "slide-geometry-harness-workflow-state.json"

LEVEL_ORDER = {
    "skip": 0,
    "lite": 1,
    "full": 2,
}

STEP_SEQUENCE = (
    "page_00",
    "page_01",
    "page_02",
    "page_03",
    "page_04",
    "page_04_data",
    "page_05",
    "page_06",
    "page_07",
    "page_08",
    "page_09",
    "page_09_img",
    "page_10",
    "page_11",
    "page_12",
    "page_13",
    "page_14",
    "page_13_img",
    "page_15_img",
    "page_15",
    "page_16",
    "page_17",
    "page_18",
    "page_18_img",
    "page_19",
    "page_21",
    "page_22",
    "page_24",
    "page_25",
    "page_26",
    "page_27",
    "page_28",
    "page_29",
    "page_30",
    "page_31",
    "page_32",
    "page_33",
)
STEP_INDEX = {step_id: index for index, step_id in enumerate(STEP_SEQUENCE)}
PAGE_TOKEN_RE = re.compile(
    r"page[\s_-]?(\d{1,2})(?:[\s_-]?(img|data))?",
    re.IGNORECASE,
)
PAGE_SCENE_RE = re.compile(
    r"Page(\d{2})(?:Scene)?(?:[\s_-]?(img|data))?",
    re.IGNORECASE,
)
DOC_PAGE_RE = re.compile(r"Docs/剧本/(\d{2})-", re.IGNORECASE)
CHINESE_PAGE_RE = re.compile(r"第\s*([零〇一二两三四五六七八九十\d]+)\s*页")
CHINESE_DIGITS = {
    "零": 0,
    "〇": 0,
    "一": 1,
    "二": 2,
    "两": 2,
    "三": 3,
    "四": 4,
    "五": 5,
    "六": 6,
    "七": 7,
    "八": 8,
    "九": 9,
}

FULL_FILE_PREFIXES = (
    "SlideApp/src/remotion/",
    "SlideApp/src/harness/slide-geometry/",
    "Docs/剧本/",
    "Docs/剧本/workloads/",
)

LITE_FILE_PREFIXES = (
    "SlideApp/src/storyboard-data/",
    "SlideApp/src/components/NotesPanel",
    "SlideApp/src/components/ProgressBubbles",
    "SlideApp/src/app.css",
    "Docs/PPT",
)

FULL_PROMPT_KEYWORDS = (
    "动画",
    "过渡",
    "几何",
    "geometry",
    "layout",
    "node",
    "edge",
    "timeline",
    "时长",
    "embed.ts",
    "shared element",
)

FULL_LAYOUT_HINT_KEYWORDS = (
    "左右两半",
    "左右布局",
    "左边是",
    "右边是",
    "左侧是",
    "右侧是",
    "上下可以",
    "上下拉",
    "拉高",
    "拉宽",
    "更宽",
    "更高",
    "更大",
    "位置不够",
    "字更大",
)

FULL_SLIDE_SIGNAL_KEYWORDS = (
    "page",
    "第",
    "页",
    "包体",
    "内存",
    "shadercode",
    "pso",
)

LITE_PROMPT_KEYWORDS = (
    "左边卡片",
    "左侧卡片",
    "左卡",
    "文案",
    "copy",
    "caption",
    "notes",
    "重点",
    "标题",
    "文字",
)


@dataclass
class GateDecision:
    level: str
    reasons: list[str]
    required_checks: list[str]
    commands: list[str]
    prompt: str
    files: list[str]
    created_at: str
    focus_step_ids: list[str] = field(default_factory=list)
    mechanical_review_from_step: str | None = None


def choose_higher_level(left: str, right: str) -> str:
    return left if LEVEL_ORDER[left] >= LEVEL_ORDER[right] else right


def normalize_path(path: str) -> str:
    return path.replace("\\", "/").strip()


def canonicalize_step_id(page_number: int, suffix: str | None = None) -> str | None:
    base_step = f"page_{page_number:02d}"
    if suffix:
        candidate = f"{base_step}_{suffix.lower()}"
        if candidate in STEP_INDEX:
            return candidate
    return base_step if base_step in STEP_INDEX else None


def parse_chinese_page_number(token: str) -> int | None:
    normalized = token.strip().replace("兩", "两")
    if not normalized:
        return None
    if normalized.isdigit():
        return int(normalized)
    if normalized == "十":
        return 10
    if "十" in normalized:
        tens_raw, ones_raw = normalized.split("十", 1)
        tens = 1 if tens_raw == "" else CHINESE_DIGITS.get(tens_raw)
        ones = 0 if ones_raw == "" else CHINESE_DIGITS.get(ones_raw)
        if tens is None or ones is None:
            return None
        return tens * 10 + ones

    value = 0
    for char in normalized:
        digit = CHINESE_DIGITS.get(char)
        if digit is None:
            return None
        value = value * 10 + digit
    return value


def sort_step_ids(step_ids: set[str]) -> list[str]:
    return sorted(step_ids, key=lambda step_id: STEP_INDEX[step_id])


def extract_step_ids_from_text(text: str) -> list[str]:
    step_ids: set[str] = set()
    for match in PAGE_TOKEN_RE.finditer(text):
        step_id = canonicalize_step_id(
            int(match.group(1)),
            match.group(2),
        )
        if step_id:
            step_ids.add(step_id)

    for match in CHINESE_PAGE_RE.finditer(text):
        page_number = parse_chinese_page_number(match.group(1))
        if page_number is None:
            continue
        step_id = canonicalize_step_id(page_number)
        if step_id:
            step_ids.add(step_id)

    return sort_step_ids(step_ids)


def extract_step_ids_from_path(path: str) -> list[str]:
    step_ids: set[str] = set()
    for match in PAGE_TOKEN_RE.finditer(path):
        step_id = canonicalize_step_id(
            int(match.group(1)),
            match.group(2),
        )
        if step_id:
            step_ids.add(step_id)

    for match in PAGE_SCENE_RE.finditer(path):
        step_id = canonicalize_step_id(
            int(match.group(1)),
            match.group(2),
        )
        if step_id:
            step_ids.add(step_id)

    doc_match = DOC_PAGE_RE.search(path)
    if doc_match:
        page_number = int(doc_match.group(1))
        lowered = path.lower()
        suffix = None
        if "_img" in lowered or "-img" in lowered:
            suffix = "img"
        elif "_data" in lowered or "-data" in lowered:
            suffix = "data"
        step_id = canonicalize_step_id(page_number, suffix)
        if step_id:
            step_ids.add(step_id)

    return sort_step_ids(step_ids)


def infer_focus_step_ids(prompt: str, files: list[str]) -> list[str]:
    step_ids: set[str] = set(extract_step_ids_from_text(prompt))
    for path in files:
        step_ids.update(extract_step_ids_from_path(path))
    return sort_step_ids(step_ids)


def classify_prompt_level(prompt: str) -> tuple[str, list[str]]:
    normalized = prompt.lower().strip()
    reasons: list[str] = []

    for keyword in FULL_PROMPT_KEYWORDS:
        if keyword in normalized:
            reasons.append(f"prompt matched full keyword: {keyword}")
            return "full", reasons

    layout_hits = [
        keyword for keyword in FULL_LAYOUT_HINT_KEYWORDS if keyword in normalized
    ]
    slide_hits = [
        keyword for keyword in FULL_SLIDE_SIGNAL_KEYWORDS if keyword in normalized
    ]
    if layout_hits and slide_hits:
        reasons.append(
            "prompt matched full layout+slide signal: "
            f"layout={','.join(layout_hits[:3])}; slide={','.join(slide_hits[:3])}"
        )
        return "full", reasons

    for keyword in LITE_PROMPT_KEYWORDS:
        if keyword in normalized:
            reasons.append(f"prompt matched lite keyword: {keyword}")
            return "lite", reasons

    return "skip", reasons


def classify_file_level(files: list[str]) -> tuple[str, list[str]]:
    reasons: list[str] = []
    level = "skip"

    for raw_path in files:
        path = normalize_path(raw_path)

        if any(path.startswith(prefix) for prefix in FULL_FILE_PREFIXES):
            reasons.append(f"file matched full prefix: {path}")
            level = choose_higher_level(level, "full")
            continue

        if any(path.startswith(prefix) for prefix in LITE_FILE_PREFIXES):
            reasons.append(f"file matched lite prefix: {path}")
            level = choose_higher_level(level, "lite")

    return level, reasons


def build_stop_plan(level: str, repo_root: Path) -> list[str]:
    if level == "skip":
        return []

    if level == "lite":
        return [
            "python3 scripts/slide-geometry-harness/audit_storyboard_sync.py",
        ]

    return [
        "python3 scripts/slide-geometry-harness/audit_transition_timings.py",
        "python3 scripts/slide-geometry-harness/audit_storyboard_sync.py",
    ]


def build_mechanical_review_command(decision: GateDecision) -> str | None:
    if decision.level != "full":
        return None

    from_step = decision.mechanical_review_from_step or "page_19"
    return f"npm --silent --prefix SlideApp run review:mechanical -- --from {from_step}"


def build_required_checks(level: str) -> list[str]:
    if level == "skip":
        return []

    if level == "lite":
        return [
            "Run storyboard/doc sync audit before claiming done.",
        ]

    return [
        "Run transition timing audit before claiming done.",
        "Run storyboard/doc sync audit before claiming done.",
        "Run formal-page mechanical review summary before claiming done.",
    ]


def classify_workflow(prompt: str = "", files: list[str] | None = None) -> GateDecision:
    file_list = files or []
    prompt_level, prompt_reasons = classify_prompt_level(prompt)
    file_level, file_reasons = classify_file_level(file_list)
    level = choose_higher_level(prompt_level, file_level)
    focus_step_ids = infer_focus_step_ids(prompt, file_list)
    mechanical_review_from_step = None
    if level == "full":
        mechanical_review_from_step = (
            focus_step_ids[0] if focus_step_ids else "page_19"
        )

    reasons = prompt_reasons + file_reasons
    if not reasons:
        reasons.append("No slide-harness signal found in prompt or files.")

    commands = build_stop_plan(level, Path.cwd())

    return GateDecision(
        level=level,
        reasons=reasons,
        required_checks=build_required_checks(level),
        commands=commands,
        prompt=prompt,
        files=file_list,
        created_at=datetime.now(timezone.utc).isoformat(),
        focus_step_ids=focus_step_ids,
        mechanical_review_from_step=mechanical_review_from_step,
    )


def write_state(state_path: Path, decision: GateDecision) -> None:
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(asdict(decision), ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )


def load_state(state_path: Path) -> GateDecision:
    payload = json.loads(state_path.read_text(encoding="utf-8"))
    return GateDecision(**payload)


def default_state_path(repo_root: Path) -> Path:
    return repo_root / ".git" / STATE_FILENAME


def run_commands(commands: list[str], repo_root: Path) -> tuple[list[dict[str, object]], bool]:
    results: list[dict[str, object]] = []
    failed = False

    for command in commands:
        result = run_command(command, repo_root)
        results.append(result)
        if int(result["returncode"]) != 0:
            failed = True

    return results, failed


def run_command(command: str, repo_root: Path) -> dict[str, object]:
    completed = subprocess.run(
        command,
        shell=True,
        cwd=repo_root,
        text=True,
        capture_output=True,
    )
    return {
        "command": command,
        "returncode": completed.returncode,
        "stdout": completed.stdout,
        "stderr": completed.stderr,
    }


def build_codex_user_prompt_submit_context(
    decision: GateDecision,
    state_path: Path,
) -> str:
    lines = [
        f"Slide harness workflow gate classified this repo task as `{decision.level}`.",
        "Required before completion:",
    ]
    lines.extend(f"- {check}" for check in decision.required_checks)
    if decision.commands:
        lines.append("Required commands:")
        lines.extend(f"- {command}" for command in decision.commands)
    review_command = build_mechanical_review_command(decision)
    if review_command:
        lines.append("Mechanical review command:")
        lines.append(f"- {review_command}")
    lines.append(
        "The Codex Stop hook will enforce these checks automatically."
    )
    lines.append(f"State file: {state_path}")
    return "\n".join(lines)


def summarize_command_output(text: str, max_lines: int = 12) -> str:
    stripped = text.strip()
    if not stripped:
        return ""
    lines = stripped.splitlines()
    if len(lines) <= max_lines:
        return "\n".join(lines)
    kept = "\n".join(lines[:max_lines])
    remaining = len(lines) - max_lines
    return f"{kept}\n... ({remaining} more lines)"


def build_codex_stop_failure_reason(
    decision: GateDecision,
    results: list[dict[str, object]],
) -> str:
    lines = [
        f"Slide harness workflow gate classified this turn as `{decision.level}` and the required audits failed.",
        "Fix the failing audits below, rerun the required audits, and only then finish the turn.",
    ]

    for result in results:
        if int(result["returncode"]) == 0:
            continue
        lines.append(
            f"Command: {result['command']} (exit {result['returncode']})"
        )
        stdout_summary = summarize_command_output(str(result["stdout"]))
        stderr_summary = summarize_command_output(str(result["stderr"]))
        if stdout_summary:
            lines.append("stdout:")
            lines.append(stdout_summary)
        if stderr_summary:
            lines.append("stderr:")
            lines.append(stderr_summary)

    return "\n".join(lines)


def build_mechanical_review_failure_reason(
    review_summary: dict[str, object],
) -> str:
    blocker_pages = list(review_summary.get("blocker_pages") or [])
    lines = [
        "Formal-page mechanical review found blocker pages computed by the existing geometry harness.",
        "Fix the blocker pages below, rerun the mechanical review summary, and only then finish the turn.",
    ]

    for blocker in blocker_pages[:3]:
        if not isinstance(blocker, dict):
            continue
        step_id = blocker.get("stepId", "<unknown>")
        review_source = blocker.get("reviewSource", "unknown")
        mechanical_score = blocker.get("mechanicalScore", "unknown")
        verdict = blocker.get("verdict", "unknown")
        lines.append(
            f"Page {step_id} ({review_source}) -> {mechanical_score} -> {verdict}"
        )
        for fix in list(blocker.get("topFixes") or [])[:2]:
            lines.append(f"Fix: {fix}")

    return "\n".join(lines)


def collect_missing_focus_review_steps(
    decision: GateDecision,
    review_summary: dict[str, object] | None,
) -> list[tuple[str, str]]:
    if not review_summary:
        return []
    if "page_results" not in review_summary:
        return []
    focus_steps = set(decision.focus_step_ids or [])
    if not focus_steps:
        return []

    page_results = list(review_summary.get("page_results") or [])
    result_by_step = {
        str(entry.get("stepId")): entry
        for entry in page_results
        if isinstance(entry, dict) and entry.get("stepId")
    }
    missing_steps: list[tuple[str, str]] = []
    for step_id in sort_step_ids(focus_steps):
        entry = result_by_step.get(step_id)
        if entry is None:
            missing_steps.append((step_id, "not_reviewed"))
            continue
        if str(entry.get("status")) == "missing_sketch":
            missing_steps.append((step_id, "missing_sketch"))
    return missing_steps


def build_missing_focus_review_failure_reason(
    missing_steps: list[tuple[str, str]],
) -> str:
    lines = [
        "Formal-page mechanical review did not produce usable review surfaces for the affected steps.",
        "Add or repair the missing review surfaces below, rerun the mechanical review summary, and only then finish the turn.",
    ]
    for step_id, reason in missing_steps[:5]:
        lines.append(f"Page {step_id} -> {reason}")
    return "\n".join(lines)


def build_codex_stop_output(
    decision: GateDecision,
    results: list[dict[str, object]],
    stop_hook_active: bool,
    review_summary: dict[str, object] | None = None,
    review_error: str | None = None,
) -> dict[str, object]:
    failed_results = [
        result for result in results if int(result["returncode"]) != 0
    ]
    if not failed_results:
        if review_error:
            failure_reason = (
                "Slide harness audits passed, but the formal-page mechanical review "
                f"summary failed.\n{review_error}"
            )
            if stop_hook_active:
                return {
                    "continue": False,
                    "stopReason": failure_reason,
                    "systemMessage": failure_reason,
                }
            return {
                "decision": "block",
                "reason": failure_reason,
            }

        if review_summary and bool(review_summary.get("has_blocker_pages")):
            failure_reason = build_mechanical_review_failure_reason(review_summary)
            if stop_hook_active:
                return {
                    "continue": False,
                    "stopReason": failure_reason,
                    "systemMessage": failure_reason,
                }
            return {
                "decision": "block",
                "reason": failure_reason,
            }

        missing_focus_steps = collect_missing_focus_review_steps(
            decision,
            review_summary,
        )
        if missing_focus_steps:
            failure_reason = build_missing_focus_review_failure_reason(
                missing_focus_steps,
            )
            if stop_hook_active:
                return {
                    "continue": False,
                    "stopReason": failure_reason,
                    "systemMessage": failure_reason,
                }
            return {
                "decision": "block",
                "reason": failure_reason,
            }

        return {"continue": True}

    failure_reason = build_codex_stop_failure_reason(decision, failed_results)
    if stop_hook_active:
        return {
            "continue": False,
            "stopReason": (
                "Slide harness audits are still failing after one Stop-hook "
                f"continuation.\n{failure_reason}"
            ),
            "systemMessage": failure_reason,
        }

    return {
        "decision": "block",
        "reason": failure_reason,
    }


def parse_mechanical_review_summary(stdout: str) -> dict[str, object]:
    payload = json.loads(stdout)
    if not isinstance(payload, dict):
        raise ValueError("Mechanical review summary must be a JSON object.")
    return payload


def run_mechanical_review(
    decision: GateDecision,
    repo_root: Path,
) -> tuple[dict[str, object] | None, dict[str, object] | None, str | None]:
    command = build_mechanical_review_command(decision)
    if not command:
        return None, None, None

    result = run_command(command, repo_root)
    if int(result["returncode"]) != 0:
        lines = [f"Command: {command} (exit {result['returncode']})"]
        stdout_summary = summarize_command_output(str(result["stdout"]))
        stderr_summary = summarize_command_output(str(result["stderr"]))
        if stdout_summary:
            lines.append("stdout:")
            lines.append(stdout_summary)
        if stderr_summary:
            lines.append("stderr:")
            lines.append(stderr_summary)
        return result, None, "\n".join(lines)

    try:
        summary = parse_mechanical_review_summary(str(result["stdout"]))
    except Exception as error:
        lines = [
            f"Command: {command} returned invalid JSON.",
            str(error),
        ]
        stdout_summary = summarize_command_output(str(result["stdout"]))
        if stdout_summary:
            lines.append("stdout:")
            lines.append(stdout_summary)
        return result, None, "\n".join(lines)

    return result, summary, None


def load_codex_hook_input() -> dict[str, object]:
    raw = sys.stdin.read()
    if not raw.strip():
        return {}
    payload = json.loads(raw)
    if not isinstance(payload, dict):
        raise ValueError("Expected Codex hook JSON object on stdin.")
    return payload


def resolve_repo_root_from_hook_input(payload: dict[str, object]) -> Path:
    cwd_value = payload.get("cwd")
    cwd = Path(str(cwd_value)).resolve() if cwd_value else Path.cwd()
    completed = subprocess.run(
        ["git", "-C", str(cwd), "rev-parse", "--show-toplevel"],
        text=True,
        capture_output=True,
    )
    if completed.returncode == 0 and completed.stdout.strip():
        return Path(completed.stdout.strip()).resolve()
    return cwd


def classify_command(args: argparse.Namespace) -> int:
    repo_root = args.repo_root.resolve()
    files = [normalize_path(path) for path in args.files]
    decision = classify_workflow(prompt=args.prompt, files=files)
    decision.commands = build_stop_plan(decision.level, repo_root)

    state_path = args.state_path.resolve() if args.state_path else default_state_path(repo_root)
    if args.write_state:
        write_state(state_path, decision)

    payload = asdict(decision)
    payload["state_path"] = str(state_path) if args.write_state else None
    print(json.dumps(payload, ensure_ascii=False, indent=2))
    return 0


def stop_command(args: argparse.Namespace) -> int:
    repo_root = args.repo_root.resolve()
    state_path = args.state_path.resolve()
    decision = load_state(state_path)
    commands = build_stop_plan(decision.level, repo_root)

    if args.dry_run:
        print(
            json.dumps(
                {
                    "level": decision.level,
                    "commands": commands,
                    "mechanical_review_command": build_mechanical_review_command(
                        decision
                    ),
                    "required_checks": decision.required_checks,
                    "state_path": str(state_path),
                },
                ensure_ascii=False,
                indent=2,
            )
        )
        return 0

    results, failed = run_commands(commands, repo_root)
    review_result, review_summary, review_error = run_mechanical_review(
        decision,
        repo_root,
    )
    missing_focus_steps = collect_missing_focus_review_steps(
        decision,
        review_summary,
    )
    geometry_failed = bool(review_error) or bool(
        review_summary and review_summary.get("has_blocker_pages")
    ) or bool(missing_focus_steps)

    print(
        json.dumps(
            {
                "level": decision.level,
                "results": results,
                "mechanical_review": {
                    "result": review_result,
                    "summary": review_summary,
                    "error": review_error,
                    "missing_focus_steps": missing_focus_steps,
                },
                "state_path": str(state_path),
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 2 if failed or geometry_failed else 0


def codex_user_prompt_submit_command(args: argparse.Namespace) -> int:
    payload = load_codex_hook_input()
    repo_root = resolve_repo_root_from_hook_input(payload)
    prompt = str(payload.get("prompt") or "")
    raw_files = payload.get("files") or []
    files = [
        normalize_path(path)
        for path in raw_files
        if isinstance(path, str)
    ]
    decision = classify_workflow(prompt=prompt, files=files)
    state_path = default_state_path(repo_root)
    write_state(state_path, decision)

    if decision.level == "skip":
        return 0

    print(build_codex_user_prompt_submit_context(decision, state_path))
    return 0


def codex_stop_command(args: argparse.Namespace) -> int:
    payload = load_codex_hook_input()
    repo_root = resolve_repo_root_from_hook_input(payload)
    state_path = default_state_path(repo_root)

    if not state_path.exists():
        print(json.dumps({"continue": True}, ensure_ascii=False))
        return 0

    decision = load_state(state_path)
    commands = build_stop_plan(decision.level, repo_root)

    if not commands:
        print(json.dumps({"continue": True}, ensure_ascii=False))
        return 0

    results, _failed = run_commands(commands, repo_root)
    _review_result, review_summary, review_error = run_mechanical_review(
        decision,
        repo_root,
    )
    response = build_codex_stop_output(
        decision,
        results,
        stop_hook_active=bool(payload.get("stop_hook_active")),
        review_summary=review_summary,
        review_error=review_error,
    )

    if response.get("continue") is True:
        state_path.unlink(missing_ok=True)

    print(json.dumps(response, ensure_ascii=False))
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Flow-layer harness gate for slide work."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    classify_parser = subparsers.add_parser(
        "classify",
        help="Classify a task into skip/lite/full harness levels.",
    )
    classify_parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    classify_parser.add_argument("--prompt", default="")
    classify_parser.add_argument("--files", nargs="*", default=[])
    classify_parser.add_argument("--write-state", action="store_true")
    classify_parser.add_argument("--state-path", type=Path)
    classify_parser.set_defaults(handler=classify_command)

    stop_parser = subparsers.add_parser(
        "stop",
        help="Run stop-phase harness audits from a persisted state file.",
    )
    stop_parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    stop_parser.add_argument("--state-path", type=Path, required=True)
    stop_parser.add_argument("--dry-run", action="store_true")
    stop_parser.set_defaults(handler=stop_command)

    codex_prompt_parser = subparsers.add_parser(
        "codex-user-prompt-submit",
        help="Adapt the workflow gate to the Codex UserPromptSubmit hook.",
    )
    codex_prompt_parser.set_defaults(handler=codex_user_prompt_submit_command)

    codex_stop_parser = subparsers.add_parser(
        "codex-stop",
        help="Adapt the workflow gate to the Codex Stop hook.",
    )
    codex_stop_parser.set_defaults(handler=codex_stop_command)

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)
    return args.handler(args)


if __name__ == "__main__":
    sys.exit(main())
