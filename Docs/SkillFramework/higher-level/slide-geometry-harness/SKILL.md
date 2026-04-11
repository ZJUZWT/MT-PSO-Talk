---
name: slide-geometry-harness
description: Use when iterating SlideApp slide layouts and you need a browser-runnable sketch that mirrors a real finished slide, a screenshot from the visible browser result, and blind dual-critic review before more edits.
---

# Slide Geometry Harness

This skill is the slide-specific orchestration layer for `SlideApp` geometry iteration.
It is a higher-level skill, not a disguised `drawio` review loop.

It composes:

- textual geometry contract drafting
- browser-runnable sketch generation
- latest-only screenshot capture
- fact-bound geometry metrics
- two blind critics
- lesson harvesting when durable workflow rules or user preferences emerge

Use this skill for slide geometry work, not for general UI review.

Keep the mechanics physically separated:

- runtime sketch code under `SlideApp/src/harness/slide-geometry/`
- harness scripts under `scripts/slide-geometry-harness/`

If the job is only "capture what Edge shows and judge whether it looks good", use `edge-image-aesthetic-review` instead.

## When to Use

Use this skill when:

- the user has a rough slide idea but not a polished sketch
- the user already has a finished or semi-finished slide and wants the sketch to mirror that page first
- the main problem is single-page layout, spacing, or line quality
- the draft must run in the real `SlideApp` browser shell
- the builder should not be allowed to self-grade

Do not use this skill for:

- final polish or production styling
- general website or product UI critique
- multi-page continuity as the primary problem

## Required Workflow

1. If a finished slide already exists, mirror that slide's real structure into the sketch first. Do not start from an abstract demo surrogate.
2. Before rendering anything, write a full page contract with `write-slide-geometry-contract`.
3. The contract must enumerate every node, every edge, every line label, the angle family, the spatial alignments, and the numbered acceptance checklist.
4. If the page contract is stored as a markdown script file, keep it under `Docs/剧本/`. If it is a sketch mirror, mark the filename with `草图镜像-<sketch-id>`. That file must also include the latest screenshot, a three-layer review summary, and auditable node and edge review tables.
5. The three-layer review summary must score:
   - `整体布局 Review`
   - `模块空间结构 Review`
   - `单节点与单边 Review`
6. The node and edge review tables must contain Chinese script descriptions when feasible, followed by review viewpoint and review score columns.
7. Only after the contract is explicit may you build or revise a sketch under `SlideApp/src/harness/slide-geometry/`.
8. Expose the sketch through the existing `SlideApp` port with a concrete URL.
9. Capture the latest rendered result using the SlideApp browser export path first:
   - primary automation path: `browser-api` capture of the real SlideApp page or stage
   - manual reality check: current front browser window capture when you need to compare against what the user literally sees on screen
   - fallback: `surface=stage` headless screenshot only when browser export is unavailable
10. Update the page script markdown screenshot, the three-layer review summary, and both review tables after capture, before critic merge.
10.5. Keep each page script responsible only for its own scores and review facts. Do not auto-propagate numeric judgments from one page into another page's ledger.
11. Extract facts and metrics before any score is allowed.
12. Send the latest artifact bundle to two blind critics in parallel:
   - `Art Critic`
   - `Geometry Critic`
13. Critics see the latest screenshot plus the contract. They do not see builder self-justification.
14. Critics must score the three review layers separately before any merged verdict:
   - `整体布局 Review`
   - `模块空间结构 Review`
   - `单节点与单边 Review`
15. Merge both reviews into one next-step verdict.
16. If the user explicitly asks to remember, persist, or write the lesson into skill or memory, invoke `conversation-lesson-harvester` before closing the task.
17. Stop after at most 3 automatic rounds, or earlier if the stop rule passes.

## Capture Rule

For agent automation, the default export path is the browser-native SlideApp capture API.

Use:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "<slide-url>"`

For stage-only export:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope stage --url "<slide-url>"`

Use front-window capture when the question is specifically about what the user literally saw in the browser window, including shell chrome or mismatch reports.

Do not fall back to screen-region capture when the browser-native export path is available.

Use front-window or headless only when:

- browser export is unavailable
- browser export failed
- you need a second source to compare with the visible browser shell

Recommended fallback:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode headless-stage --url "<slide-url>"`

## Required Output

Every completed round must include:

- `Element contract`
- `Page script markdown`
- `URL`
- `Latest screenshot`
- `Capture provenance`
- `Capture size`
- `Facts`
- `Metrics`
- `Three-layer review summary`
- `Art critic verdict`
- `Geometry critic verdict`
- `Top 3 fixes`

## Hard Bans

- Do not let the builder agent score its own sketch.
- Do not let critics read the builder's self-justification.
- Do not review an old screenshot when a newer one exists.
- Do not start from a fake generic diagram when the real slide already exists.
- Do not treat a vague sketch as a substitute for a node/edge contract.
- Do not render before nodes, edges, line labels, and alignments are enumerated.
- Do not keep screenshot, node tables, and edge tables in different sources of truth.
- Do not keep screenshot, three-layer review summary, node tables, and edge tables in different sources of truth.
- Do not write a page script markdown file that has edges reviewed in prose but nodes reviewed in tables; keep the format symmetric and auditable.
- Do not skip the `整体布局 Review` score just because the local routing looks correct.
- Do not keep using screen-region capture when browser-native export already works.
- Do not omit capture bounds or browser-chrome status when reporting provenance.
- Do not emit side-by-side comparison renders by default.
- Do not use freeform scores with no fact layer.

## Reference

See [references/loop-contract.md](references/loop-contract.md) for the control loop and [references/critic-prompts.md](references/critic-prompts.md) for the two blind critic roles.
