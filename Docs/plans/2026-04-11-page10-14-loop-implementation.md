# Page 10-14 Loop Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** add a formal page 10 transition scene plus sketchable pages 10-14 for the PSO loop chapter.

**Architecture:** extend the storyboard and step timeline to pages 10-14, implement page 10 as a new remotion scene that internally replays a page 05-like bridge before reforming into the new cook/runtime stop frame, and register geometry sketches for pages 10-14 so each page has a concrete review URL. Keep page 11-14 in sketch mode first to reduce overnight risk.

**Tech Stack:** React, Remotion, Vitest, SlideApp geometry sketch harness

---

### Task 1: Extend step metadata to pages 10-14

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`

**Step 1: Write the failing test**

Add tests that expect:

- `page_10` to `page_14` exist in the storyboard sequence
- remotion frame map resolves those new steps
- step navigation can accept those step ids

**Step 2: Run test to verify it fails**

Run:

```bash
npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/App.test.tsx
```

**Step 3: Write minimal implementation**

- extend `StoryStepId`
- append five new storyboard steps
- add frame numbers for pages 10-14
- extend remotion step sequence
- expose page frame constants

**Step 4: Run test to verify it passes**

Run the same command and confirm the new assertions pass.

### Task 2: Add failing tests for page 10 stop-frame and transition anchors

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add tests that expect page 10 to:

- render a computer cluster, cook hub, `.shaderbytecode`, `.scl.csv`, and runtime-phone cluster
- keep the exclamation mark out of the final stop frame
- expose a transient `page10-answer-badge` during the page 09 -> 10 transition window

**Step 2: Run test to verify it fails**

Run:

```bash
npm test -- --run src/remotion/Composition.test.tsx -t "page 10"
```

**Step 3: Write minimal implementation**

- add `Page10Scene`
- extend `computeSceneModel` with page 09 -> 10 progress
- render transition badge and final layout

**Step 4: Run test to verify it passes**

Run the same command again.

### Task 3: Add failing tests for page 10-14 sketch registration

**Files:**
- Modify: `SlideApp/src/App.test.tsx`
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page10-r1.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page11-r1.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page12-r1.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page13-r1.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page14-r1.ts`

**Step 1: Write the failing test**

Add tests that expect:

- each sketch id resolves
- each sketch URL boots in sketch mode
- each sketch review artifact reports the intended receiver plane and primary line

**Step 2: Run test to verify it fails**

Run:

```bash
npm test -- --run src/App.test.tsx src/harness/slide-geometry/review/geometryReviewArtifact.test.ts
```

**Step 3: Write minimal implementation**

- create one contract per page
- add them to the sketch registry
- keep routing simple, orthogonal, and readable

**Step 4: Run test to verify it passes**

Run the same command again.

### Task 4: Capture and verify the new artifacts

**Files:**
- No code changes required unless capture flow reveals a regression

**Step 1: Run focused verification**

Run:

```bash
npm test -- --run src/remotion/Composition.test.tsx src/App.test.tsx src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/harness/slide-geometry/review/geometryReviewArtifact.test.ts
```

**Step 2: Capture the final page 10 scene**

Use:

```bash
bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope stage --url 'http://127.0.0.1:4173/?step=page_10' --outdir ignore/recheck-page10 --prefix page10-stage
```

**Step 3: Capture at least one sketch page**

Use:

```bash
bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope stage --url 'http://127.0.0.1:4173/?mode=sketch&sketch=page11-r1&review=1' --outdir ignore/recheck-page10 --prefix page11-sketch
```

**Step 4: Review against the contract**

Check:

- page 10 stop frame contains only the intended clusters
- page 10 answer badge is transitional rather than persistent
- each sketch has a stable review URL
- no text overflow or clipped labels
