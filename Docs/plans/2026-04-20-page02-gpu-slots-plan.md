# Page 02 GPU Slots Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add three lightweight slot-like visuals above the page 02 GPU to foreshadow configurable GPU behavior, then let them fade away before page 03 takes over with the real upper configuration band.

**Architecture:** Keep the existing `VertexData -> GPU -> Pixels` composition unchanged and add one tiny decorative layer inside `Page02Scene`. Reuse the page 02 / page 03 transition progress so the slots belong to page 02 and naturally fade out as page 03 begins.

**Tech Stack:** React, Remotion SVG scene rendering, Vitest composition tests, page script markdown

---

### Task 1: Add the page 02 GPU slot visuals

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page02Scene.tsx`

**Step 1: Add a tiny slot layout helper near the GPU**

- Define three evenly spaced slot positions centered above `GPU`.
- Keep them purely visual: no labels, no explicit connector lines.

**Step 2: Fade the slots with page 02 ownership**

- Drive slot opacity from the existing page state so they are fully visible on settled page 02.
- Multiply by `1 - settledPage23Progress` so they yield to page 03.

**Step 3: Render minimal slot shapes**

- Draw small rounded shells with a lighter inset/slit feel.
- Keep stroke/fill language aligned with the page 02 card palette.

### Task 2: Lock behavior with a focused render test

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Add an assertion for settled page 02**

- Render page 02 and assert that exactly three GPU slot markers are present.

**Step 2: Add an assertion for settled page 03**

- Render page 03 and assert that the page 02 slot markers are absent or fully faded.

### Task 3: Sync the script and storyboard text

**Files:**
- Modify: `Docs/剧本/02-第二页.md`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Update page 02 script wording**

- Record that page 02 now includes three lightweight GPU parameter slots as a visual foreshadowing element.

**Step 2: Update storyboard wording**

- Mention that page 02 already hints the GPU has configurable behavior, while page 03 performs the real expansion.

### Task 4: Verify and capture

**Files:**
- No direct file changes

**Step 1: Run focused tests**

- `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "renders page 02 as VertexData -> GPU -> pixels without GPU inner chrome|keeps page 02 -> page 03 upper band and API lines still animating halfway through"`

**Step 2: Capture the updated page**

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "http://127.0.0.1:4174/?step=page_02" --outdir "/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/ignore/browser-api-captures/20260420-page02-gpu-slots" --prefix page02-gpu-slots`

**Step 3: Run repo-required audits**

- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

