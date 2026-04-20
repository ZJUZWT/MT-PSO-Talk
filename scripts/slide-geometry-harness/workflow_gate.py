#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from pathlib import Path


STATE_FILENAME = "slide-geometry-harness-workflow-state.json"

LEVEL_ORDER = {
    "skip": 0,
    "lite": 1,
    "full": 2,
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


def choose_higher_level(left: str, right: str) -> str:
    return left if LEVEL_ORDER[left] >= LEVEL_ORDER[right] else right


def normalize_path(path: str) -> str:
    return path.replace("\\", "/").strip()


def classify_prompt_level(prompt: str) -> tuple[str, list[str]]:
    normalized = prompt.lower().strip()
    reasons: list[str] = []

    for keyword in FULL_PROMPT_KEYWORDS:
        if keyword in normalized:
            reasons.append(f"prompt matched full keyword: {keyword}")
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


def build_mechanical_review_command(level: str) -> str | None:
    if level != "full":
        return None

    return "npm --prefix SlideApp run review:mechanical -- --from page_19"


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
    review_command = build_mechanical_review_command(decision.level)
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
    command = build_mechanical_review_command(decision.level)
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
                        decision.level
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
    geometry_failed = bool(review_error) or bool(
        review_summary and review_summary.get("has_blocker_pages")
    )

    print(
        json.dumps(
            {
                "level": decision.level,
                "results": results,
                "mechanical_review": {
                    "result": review_result,
                    "summary": review_summary,
                    "error": review_error,
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
    decision = classify_workflow(prompt=prompt, files=[])
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
