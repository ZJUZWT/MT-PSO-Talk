#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


REQUIRED_SECTIONS = [
    "## Harness 公式快照",
    "## 页面评估看板（四项）",
    "## 三层 Review 总表",
    "## 节点剧本与 Review 表",
    "## 边剧本与 Review 表",
]

IGNORED_DOC_FILENAMES = {
    "00-剧本与页面对齐审计.md",
}

PLACEHOLDER_MARKERS = [
    "待补",
    "待刷新",
    "TODO",
    "请运行 `probe_transition_timeline.py",
    "当前状态：`待补`",
]


@dataclass
class DocInfo:
    path: Path
    step_id: str | None
    step_inferred_from_page_index: bool
    has_animation_section: bool
    has_screenshot: bool
    timing_state: str
    missing_sections: list[str]
    placeholder_hits: list[str]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_step_frame_map(embed_ts: Path) -> dict[str, int]:
    text = read_text(embed_ts)
    match = re.search(
        r"STEP_FRAME_MAP:\s*Record<StoryStepId,\s*number>\s*=\s*\{(.*?)\};",
        text,
        re.DOTALL,
    )
    if not match:
        raise RuntimeError(f"failed to parse STEP_FRAME_MAP in {embed_ts}")
    pairs = re.findall(r"^\s*([a-zA-Z0-9_]+)\s*:\s*(\d+)\s*,?\s*$", match.group(1), re.MULTILINE)
    if not pairs:
        raise RuntimeError(f"STEP_FRAME_MAP looks empty in {embed_ts}")
    return {k: int(v) for k, v in pairs}


def parse_step_sequence(scene_timeline_ts: Path) -> list[str]:
    text = read_text(scene_timeline_ts)
    match = re.search(
        r"REMOTION_STEP_SEQUENCE:\s*StoryStepId\[\]\s*=\s*\[(.*?)\];",
        text,
        re.DOTALL,
    )
    if not match:
        raise RuntimeError(f"failed to parse REMOTION_STEP_SEQUENCE in {scene_timeline_ts}")
    steps = re.findall(r'"([a-zA-Z0-9_]+)"', match.group(1))
    if not steps:
        raise RuntimeError(f"REMOTION_STEP_SEQUENCE looks empty in {scene_timeline_ts}")
    return steps


def newest_transition_audit_json(audit_dir: Path) -> Path:
    candidates = sorted(audit_dir.glob("transition-timing-audit-*.json"))
    if not candidates:
        raise RuntimeError(f"no transition timing audit json in {audit_dir}")
    return candidates[-1]


def parse_doc_step_id(text: str) -> str | None:
    def normalize_step_token(raw: str) -> str | None:
        token = raw.strip()
        if not token:
            return None
        if token.startswith("page_"):
            return token
        # Support compact aliases like `4_data`, `13_img`, `5`.
        alias = re.match(r"^(\d+)(?:_(data|img))?$", token)
        if alias:
            number = int(alias.group(1))
            suffix = alias.group(2)
            base = f"page_{number:02d}"
            if suffix:
                return f"{base}_{suffix}"
            return base
        return None

    # Preferred explicit metadata.
    explicit = re.search(r"Step / Sketch：`([^`]+)`", text)
    if explicit:
        normalized = normalize_step_token(explicit.group(1))
        if normalized:
            return normalized

    explicit_step = re.search(r"Step：`([^`]+)`", text)
    if explicit_step:
        normalized = normalize_step_token(explicit_step.group(1))
        if normalized:
            return normalized

    # Fallback: first page_xx-like token in title/body.
    match = re.search(r"`(page_[0-9]{2}(?:_[a-z]+)?)`", text)
    if match:
        return match.group(1)
    return None


def detect_timing_state(text: str) -> str:
    if "## 过渡动画剧本与时长审查" in text:
        if "当前状态：`待补`" in text or "请运行 `probe_transition_timeline.py" in text:
            return "placeholder"
        return "present"
    if "durationVerdict=" in text:
        return "present"
    return "missing"


def parse_doc(path: Path) -> DocInfo:
    text = read_text(path)
    missing_sections = [sec for sec in REQUIRED_SECTIONS if sec not in text]
    placeholders = [m for m in PLACEHOLDER_MARKERS if m in text]
    return DocInfo(
        path=path,
        step_id=parse_doc_step_id(text),
        step_inferred_from_page_index=False,
        has_animation_section=("## 动画剧本" in text) or ("## 过渡动画剧本与时长审查" in text),
        has_screenshot=("## Current screenshot" in text) or ("## 当前截图" in text),
        timing_state=detect_timing_state(text),
        missing_sections=missing_sections,
        placeholder_hits=placeholders,
    )


def fmt_doc_quality(doc: DocInfo) -> str:
    tags: list[str] = []
    if not doc.has_animation_section:
        tags.append("no_anim")
    if not doc.has_screenshot:
        tags.append("no_shot")
    if doc.timing_state != "present":
        tags.append(f"timing_{doc.timing_state}")
    if doc.missing_sections:
        tags.append(f"missing_sections={len(doc.missing_sections)}")
    if doc.placeholder_hits:
        tags.append("has_placeholder")
    if not tags:
        return "ok"
    return ", ".join(tags)


def iter_story_docs(script_dir: Path) -> Iterable[DocInfo]:
    for path in sorted(script_dir.glob("*.md")):
        if path.name == "README.md":
            continue
        if path.name == "_page-template.md":
            continue
        if path.name in IGNORED_DOC_FILENAMES:
            continue
        if "草图镜像" in path.name:
            continue
        if "后续优化方向备注草案" in path.name:
            continue
        yield parse_doc(path)


def page_index_from_filename(name: str) -> int | None:
    match = re.match(r"^([0-9]{2})-", name)
    if not match:
        return None
    return int(match.group(1))


def build_report(
    sequence: list[str],
    frame_map: dict[str, int],
    transitions: list[dict],
    docs: list[DocInfo],
    audit_json_path: Path,
) -> str:
    def step_alias(step: str) -> str:
        match = re.match(r"^page_(\d{2})(?:_(data|img))?$", step)
        if not match:
            return step
        number = int(match.group(1))
        suffix = match.group(2)
        if suffix:
            return f"{number}_{suffix}"
        return str(number)

    fps = 60
    doc_by_step: dict[str, list[DocInfo]] = {}
    for doc in docs:
        if doc.step_id is None:
            continue
        doc_by_step.setdefault(doc.step_id, []).append(doc)

    transition_by_from = {item["fromStepId"]: item for item in transitions}
    bad_transitions = [t for t in transitions if t.get("verdict") != "in_range"]
    missing_doc_steps: list[str] = []

    lines: list[str] = []
    lines.append("# 剧本与页面对齐审计")
    lines.append("")
    lines.append(f"- 数据来源（时间审计）：`{audit_json_path}`")
    lines.append(f"- Step 总数：`{len(sequence)}`")
    lines.append(f"- 正式动画剧本文件数：`{len(docs)}`")
    lines.append(f"- 动画时长异常转场数（verdict != in_range）：`{len(bad_transitions)}`")
    lines.append("")

    if bad_transitions:
        lines.append("## 动画异常转场")
        lines.append("")
        lines.append("| from | to | durationSec | allowedMinSec | allowedMaxSec | verdict |")
        lines.append("| --- | --- | --- | --- | --- | --- |")
        for t in bad_transitions:
            lines.append(
                f"| `{step_alias(t['fromStepId'])}` | `{step_alias(t['toStepId'])}` | `{t['durationSec']:.3f}` | `{t['allowedMinSec']:.3f}` | `{t['allowedMaxSec']:.3f}` | `{t['verdict']}` |"
            )
        lines.append("")

    lines.append("## Step 与剧本映射")
    lines.append("")
    lines.append("| 可见页序 | step | frame | time(s) | next(verdict) | 剧本文件 | 对齐状态 | 剧本质量 |")
    lines.append("| --- | --- | --- | --- | --- | --- | --- | --- |")

    for index, step in enumerate(sequence, start=1):
        frame = frame_map[step]
        sec = frame / fps
        t = transition_by_from.get(step)
        next_desc = "-"
        if t:
            next_desc = f"{step_alias(t['toStepId'])} / {t['durationSec']:.3f}s / {t['verdict']}"

        step_docs = doc_by_step.get(step, [])
        if not step_docs:
            missing_doc_steps.append(step)
            lines.append(
                f"| {index} | `{step_alias(step)}` | {frame} | {sec:.3f} | `{next_desc}` | - | `missing_doc` | - |"
            )
            continue

        for doc_idx, doc in enumerate(step_docs):
            file_label = doc.path.name
            quality = fmt_doc_quality(doc)
            align_state = "aligned" if doc.step_id == step else "step_mismatch"
            prefix = str(index) if doc_idx == 0 else ""
            inferred_tag = " (inferred)" if doc.step_inferred_from_page_index else ""
            lines.append(
                f"| {prefix} | `{step_alias(step)}` | {frame} | {sec:.3f} | `{next_desc}` | `{file_label}` | `{align_state}` | `{quality}{inferred_tag}` |"
            )

    lines.append("")
    lines.append("## 剧本文件但未匹配到 step 的条目")
    lines.append("")

    unmatched = [doc for doc in docs if doc.step_id is None or doc.step_id not in sequence]
    if not unmatched:
        lines.append("- 无")
    else:
        for doc in unmatched:
            lines.append(f"- `{doc.path.name}`（step=`{doc.step_id}`）")

    lines.append("")
    lines.append("## 结论")
    lines.append("")
    lines.append("- 如果你看到“剧本和页面对不上”，通常是 `step` 序列有插页（如 `_data`、`_img`）后，文档没有同步补齐。")
    priorities: list[str] = []
    if missing_doc_steps:
        priorities.append("`missing_doc`")
    if any(doc.timing_state == "placeholder" for doc in docs):
        priorities.append("`timing_placeholder`")
    if any(not doc.has_animation_section for doc in docs):
        priorities.append("`no_anim`")
    if any(not doc.has_screenshot for doc in docs):
        priorities.append("`no_shot`")
    if bad_transitions:
        priorities.append("`duration_verdict!=in_range`")
    if priorities:
        lines.append(f"- 以本次审计为准，优先修复：{ '、'.join(priorities) }。")
    else:
        lines.append("- 以本次审计为准，当前不存在结构性缺口。")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Audit storyboard step/page/script sync.")
    parser.add_argument(
        "--repo-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="Repository root path.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("Docs/剧本/00-剧本与页面对齐审计.md"),
        help="Output markdown path (relative to repo root when not absolute).",
    )
    parser.add_argument(
        "--audit-json",
        type=Path,
        default=None,
        help="Transition timing audit json path; newest file is used by default.",
    )
    args = parser.parse_args()

    repo_root = args.repo_root.resolve()
    embed_ts = repo_root / "SlideApp/src/remotion/embed.ts"
    scene_timeline_ts = repo_root / "SlideApp/src/remotion/sceneTimeline.ts"
    script_dir = repo_root / "Docs/剧本"
    audit_dir = repo_root / "generated/transition-timing-audit"

    sequence = parse_step_sequence(scene_timeline_ts)
    frame_map = parse_step_frame_map(embed_ts)

    audit_json = args.audit_json.resolve() if args.audit_json else newest_transition_audit_json(audit_dir)
    transitions = json.loads(read_text(audit_json)).get("transitions", [])
    docs = list(iter_story_docs(script_dir))
    for doc in docs:
        if doc.step_id is not None:
            continue
        page_index = page_index_from_filename(doc.path.name)
        if page_index is None:
            continue
        if 1 <= page_index <= len(sequence):
            doc.step_id = sequence[page_index - 1]
            doc.step_inferred_from_page_index = True

    report = build_report(sequence, frame_map, transitions, docs, audit_json)

    output = args.output if args.output.is_absolute() else repo_root / args.output
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(report + "\n", encoding="utf-8")

    print(str(output))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
