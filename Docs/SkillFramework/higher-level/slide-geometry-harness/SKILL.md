---
name: slide-geometry-harness
description: Use when iterating SlideApp slide layouts and you need a browser-runnable sketch that mirrors a real finished slide, a screenshot from the visible browser result, and blind dual-critic review before more edits.
---

# Slide Geometry Harness

This skill is the slide-specific orchestration layer for `SlideApp` geometry iteration.
It is a higher-level skill, not a disguised `drawio` review loop.

It composes:

- browser-runnable sketch generation
- latest-only screenshot capture
- fact-bound geometry metrics
- two blind critics

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
2. Convert the current page into a short page contract after the mirror pass, not before it.
3. Build or revise a sketch under `SlideApp/src/harness/slide-geometry/`.
4. Expose the sketch through the existing `SlideApp` port with a concrete URL.
5. Capture the latest rendered result using the SlideApp browser export path first:
   - primary automation path: `browser-api` capture of the real SlideApp page or stage
   - manual reality check: current front browser window capture when you need to compare against what the user literally sees on screen
   - fallback: `surface=stage` headless screenshot only when browser export is unavailable
6. Extract facts and metrics before any score is allowed.
7. Send the latest artifact bundle to two blind critics in parallel:
   - `Art Critic`
   - `Geometry Critic`
8. Merge both reviews into one next-step verdict.
9. Stop after at most 3 automatic rounds, or earlier if the stop rule passes.

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

- `URL`
- `Latest screenshot`
- `Capture provenance`
- `Capture size`
- `Facts`
- `Metrics`
- `Art critic verdict`
- `Geometry critic verdict`
- `Top 3 fixes`

## Hard Bans

- Do not let the builder agent score its own sketch.
- Do not let critics read the builder's self-justification.
- Do not review an old screenshot when a newer one exists.
- Do not start from a fake generic diagram when the real slide already exists.
- Do not keep using screen-region capture when browser-native export already works.
- Do not omit capture bounds or browser-chrome status when reporting provenance.
- Do not emit side-by-side comparison renders by default.
- Do not use freeform scores with no fact layer.

## Reference

See [references/loop-contract.md](references/loop-contract.md) for the control loop and [references/critic-prompts.md](references/critic-prompts.md) for the two blind critic roles.
