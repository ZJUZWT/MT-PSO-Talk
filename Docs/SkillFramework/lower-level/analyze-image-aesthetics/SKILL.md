---
name: analyze-image-aesthetics
description: Use when you need a structured aesthetic review of one actual image, frame, or slide screenshot, with scoring and concrete next fixes.
---

# Analyze Image Aesthetics

This skill reviews one real image artifact, not code, memory, or intent.

If there is no actual image file or attached image, stop and get one first.

## When to Use

Use this skill when:

- the user asks whether a page, frame, or image "looks right"
- you need a beauty and readability review of one rendered visual
- a higher-level review skill already captured a screenshot and now needs analysis

Do not use this skill for implementation review, code review, or page-to-page animation logic without an actual rendered image.

For `SlideApp` geometry sketch iteration, prefer the dedicated higher-level `slide-geometry-harness` so the builder, capture loop, and blind critics stay separated.

## Required Workflow

1. Inspect the actual image.
2. State what the target subject is and whether non-subject chrome is present.
3. Run a blocker scan before praising anything.
4. Score the image using the rubric in [references/aesthetic-rubric.md](references/aesthetic-rubric.md).
5. Report blockers first, then strengths, then the smallest high-leverage fixes.

## Required Output

Always include:

- `Blockers`
- `Overall verdict`
- a score table with:
  - `空间美感`
  - `视觉层级`
  - `构图平衡`
  - `信息密度`
  - `可读性`
  - `演讲适配度`
- one short reason per score
- one next fix per score
- `Top 3 fixes`

## Hard Bans

- Do not analyze from memory when the image is available.
- Do not say "looks good" without naming concrete evidence in the image.
- Do not bury blockers under general praise.
- Do not mark a page as polished if any core score is below `7`.
- Do not confuse browser chrome with the design target unless the chrome is part of the task.
- Do not substitute this generic skill for the slide-specific dual-critic harness when the task is really geometry iteration inside `SlideApp`.

## Reference

See [references/aesthetic-rubric.md](references/aesthetic-rubric.md) for scoring anchors and blocker checks.
