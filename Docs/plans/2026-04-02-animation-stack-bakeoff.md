# Animation Stack Bakeoff Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Produce three directly comparable PSO concept-animation demos using Motion Canvas, Remotion, and Manim so the user can choose the best stack for the final PPT assets.

**Architecture:** Keep one shared storyboard and one shared scene vocabulary across all stacks: `A`, `Material A/B/C`, `ShaderCodeLib`, `f(x)`, `ShaderCode`, `Pipeline State`, and `B`. Build one minimal but polished prototype per stack under an isolated `Docs/prototypes/` subtree, then render one MP4 and one poster/preview per stack for side-by-side comparison.

**Tech Stack:** Homebrew `ffmpeg`, Python `manim`, Node.js, Motion Canvas, Remotion, local render scripts

---

### Task 1: Install render dependencies

**Files:**
- Create: `Docs/prototypes/README.md`

**Step 1: Install `ffmpeg`**

Run: `brew install ffmpeg`
Expected: Homebrew finishes successfully and `ffmpeg -version` reports a version.

**Step 2: Install `manim`**

Run: `python3 -m pip install --user manim`
Expected: `python3 -m manim --version` reports a version.

**Step 3: Record environment notes**

Write a short dependency note in `Docs/prototypes/README.md` covering which stacks were installed and how to rerender them.

### Task 2: Scaffold Motion Canvas prototype

**Files:**
- Create: `Docs/prototypes/motion-canvas-demo/...`

**Step 1: Scaffold the project**

Run: `npm exec @motion-canvas/create@latest`
Expected: A new Motion Canvas project is created under `Docs/prototypes/motion-canvas-demo`.

**Step 2: Replace the default scene with the shared storyboard**

Implement the PSO concept scene using Motion Canvas primitives and timing.

**Step 3: Render preview assets**

Run the project render command and export an MP4 plus a poster/screenshot.

### Task 3: Scaffold Remotion prototype

**Files:**
- Create: `Docs/prototypes/remotion-demo/...`

**Step 1: Scaffold the project**

Run: `npm init video@latest`
Expected: A new Remotion project is created under `Docs/prototypes/remotion-demo`.

**Step 2: Replace the default composition with the shared storyboard**

Implement the same scene as SVG/React components with timeline-driven transitions.

**Step 3: Render preview assets**

Run Remotion render to export an MP4 plus a poster/screenshot.

### Task 4: Scaffold Manim prototype

**Files:**
- Create: `Docs/prototypes/manim-demo/pso_concept_demo.py`
- Create: `Docs/prototypes/manim-demo/render_preview.sh`

**Step 1: Write the Manim scene**

Implement the shared storyboard as one short scene focused on structural clarity rather than long narration.

**Step 2: Render preview assets**

Run a local Manim render command and export one MP4 plus one representative poster frame.

### Task 5: Build comparison output

**Files:**
- Create: `Docs/prototypes/compare_results.md`

**Step 1: Collect outputs**

List absolute output paths for each stack’s MP4 and poster.

**Step 2: Summarize tradeoffs**

Write a short comparison covering:
- visual quality
- iteration speed
- code ergonomics
- suitability for “knowledge evolution” technical-talk animation

**Step 3: Verify all outputs exist**

Run file existence checks for all generated previews and include exact commands in the summary.
