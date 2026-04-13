# PSO Placeholder Insert Pages Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** insert three placeholder explanation pages after the current runtime landing, runtime return, and stable close-loop beats, while shifting the old page numbering forward and preserving the main flow chapter.

**Architecture:** extend the storyboard and step-id system from pages 13-15 to pages 13-18, keep the existing loop scene as the shared flow backbone, and add three lightweight onepage explanation states that use a common fade/shrink insert transition. The first pass should optimize for clean sequencing and placeholder readability, not final polish.

**Tech Stack:** React, Remotion, SlideApp storyboard metadata, Vitest

---

### Task 1: Extend step ids and storyboard metadata to pages 16-18

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`
- Test: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Test: `SlideApp/src/remotion/sceneTimeline.test.ts`

**Step 1: Write the failing test**

Add assertions that:

- `page_16`, `page_17`, and `page_18` are valid `StoryStepId` values
- storyboard order becomes `page_13 -> page_14 -> page_15 -> page_16 -> page_17 -> page_18`
- remotion frame lookup resolves all six pages in increasing order

**Step 2: Run test to verify it fails**

Run:

```bash
cd SlideApp && npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts
```

**Step 3: Write minimal implementation**

- extend the step id union to `page_18`
- shift old page 14/15 semantics in the storyboard text
- append new placeholder storyboard entries for `page_14`, `page_16`, `page_18`
- update remotion frame map and step sequence

**Step 4: Run test to verify it passes**

Run the same command again.

### Task 2: Write page scripts for the three placeholder pages

**Files:**
- Create: `Docs/剧本/14-第十四页-正式动画.md`
- Create or rewrite: `Docs/剧本/16-第十六页-正式动画.md`
- Create or rewrite: `Docs/剧本/18-第十八页-正式动画.md`

**Step 1: Draft page 14 script**

Write the placeholder script for runtime PSO collection with:

- page goal
- content card layout
- `OpenGL` vs `Metal` comparison
- inserted-page transition rule

**Step 2: Draft page 16 script**

Write the placeholder script for `expand / build` and file semantics with:

- four file cards
- `OpenGL` notes where required
- bottom takeaway strip

**Step 3: Draft page 18 script**

Write the placeholder script for precompile and compile-count comparison with:

- precompile input/process/output structure
- `OpenGL` vs `Metal` compile-count explanation

**Step 4: Verify consistency**

Check that page 14, 16, and 18 all use the same onepage transition grammar and placeholder visual language.

### Task 3: Re-map the old flow pages to new numbers

**Files:**
- Modify: `Docs/剧本/15-第十五页-正式动画.md`
- Create or rewrite: `Docs/剧本/17-第十七页-正式动画.md`
- Optionally create redirect notes in old files if needed for editorial clarity

**Step 1: Move old page 14 semantics to page 15**

Update the page script so `Phone -> rec.upipelinecache -> Computer` becomes the new `page_15`.

**Step 2: Move old page 15 semantics to page 17**

Update the stable close-loop script so it becomes the new `page_17`.

**Step 3: Verify references**

Confirm:

- each page script points to the correct previous page
- captions and manuscript text mention the new numbering
- there are no stale references to the old 13-15 chapter shape

### Task 4: Implement placeholder rendering states in the loop scene

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/model/computeSceneModel.ts`
- Test: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add tests that expect:

- `page_14`, `page_16`, and `page_18` to render onepage placeholder content
- `page_15` to still render the old runtime return semantics
- `page_17` to render the old stable close-loop semantics

**Step 2: Run test to verify it fails**

Run:

```bash
cd SlideApp && npm test -- --run src/remotion/Composition.test.tsx
```

**Step 3: Write minimal implementation**

- add three inserted-page render branches
- reuse existing scene shell and palette
- shrink/fade the underlying flow frame during onepage entry
- restore the prior flow frame before transitioning into the next flow page

**Step 4: Run test to verify it passes**

Run the same command again.

### Task 5: Verify navigation and capture the placeholder pages

**Files:**
- No new source files unless a verification failure requires a fix

**Step 1: Run focused verification**

Run:

```bash
cd SlideApp && npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx
```

**Step 2: Run build**

Run:

```bash
cd SlideApp && npm run build
```

**Step 3: Capture the new pages**

Use:

```bash
bash scripts/slide-geometry-harness/capture_review_png.sh --mode headless-stage --url 'http://127.0.0.1:4178/?step=page_14&surface=stage' --outdir ignore/recheck-page14 --prefix page14-placeholder
bash scripts/slide-geometry-harness/capture_review_png.sh --mode headless-stage --url 'http://127.0.0.1:4178/?step=page_16&surface=stage' --outdir ignore/recheck-page16 --prefix page16-placeholder
bash scripts/slide-geometry-harness/capture_review_png.sh --mode headless-stage --url 'http://127.0.0.1:4178/?step=page_18&surface=stage' --outdir ignore/recheck-page18 --prefix page18-placeholder
```

**Step 4: Sanity check the result**

Confirm:

- the inserted pages read as onepage explanation boards rather than topology pages
- numbering and previous/next-page semantics are correct
- the exit from each placeholder re-enters the main flow cleanly

Plan complete and saved to `docs/plans/2026-04-13-pso-placeholder-insert-pages-implementation.md`. Two execution options:

**1. Subagent-Driven (this session)** - I dispatch fresh subagent per task, review between tasks, fast iteration

**2. Parallel Session (separate)** - Open new session with executing-plans, batch execution with checkpoints

Which approach?
