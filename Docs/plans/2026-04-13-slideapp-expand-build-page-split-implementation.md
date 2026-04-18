# SlideApp Expand/Build Page Split Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Split the current page 16 `expand / build` explanation into two clearer pages, shift the stable-loop closeout and precompile explanation back by one page, and keep the animation/storyboard/timeline in sync.

**Architecture:** Treat the change as a storyboard-and-timeline feature first, then remap the Remotion scene beats to the new page order. Keep page 16 and page 17 as lightweight onepage explanations, restore the main stage on page 18 for the stable-loop closeout, and move the precompile explanation to a new page 19.

**Tech Stack:** TypeScript, React, Remotion, Vitest, markdown docs

---

### Task 1: Lock the new step order in tests

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/sceneTimeline.test.ts`

**Step 1: Write the failing tests**

- Change the storyboard step-order test to expect `page_19`.
- Change the storyboard narrative assertions so:
  - `page_16` is expand-only
  - `page_17` is build-only
  - `page_18` is stable-loop closeout
  - `page_19` is precompile
- Change the timeline test to expect:
  - one more step in `REMOTION_STEP_SEQUENCE`
  - new frame anchors for `page_18` and `page_19`
  - a final hold keyed off `page_19`

**Step 2: Run tests to verify they fail**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts`

Expected: failures showing the old `page_16/page_17/page_18` mapping is still in code.

**Step 3: Write minimal implementation**

- Update the step-id types and step sequence to include `page_19`.
- Update Remotion frame anchors and total duration.

**Step 4: Run tests to verify they pass**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts`

Expected: passing step-order and frame-anchor tests.

### Task 2: Rewrite storyboard metadata for pages 16-19

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Write the failing test**

- Expand the existing storyboard assertions from Task 1 until they encode the actual narrative requirements:
  - page 16 explains why `expand` needs historical `scl.csv`
  - page 17 explains `build` with current `scl.csv`
  - page 18 returns to the stable loop on the main stage
  - page 19 explains precompile

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts -t "placeholder-expanded loop chapter"`

Expected: failures on labels/manuscripts still mentioning the old combined page.

**Step 3: Write minimal implementation**

- Update the storyboard summary text.
- Rewrite `page_16`, `page_17`, `page_18`.
- Add new `page_19`.

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: storyboard tests pass with the new page semantics.

### Task 3: Split the Remotion onepage content and shift the loop closeout

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/embed.ts`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/pages/page-layout-constants.ts`

**Step 1: Write the failing tests**

- Update the focused composition tests so they expect:
  - page 16 content to be expand-only
  - page 17 content to be build-only
  - page 18 to contain the stable main-stage closeout
  - page 19 to contain precompile

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "placeholder explanation pages|stable loop"`

Expected: failures because the current scene still maps those beats to pages 16-18 only.

**Step 3: Write minimal implementation**

- Add a new page-19 anchor and reveal window.
- Turn current `Page16Placeholder` into expand-only.
- Add a new `Page17Placeholder` for build-only.
- Shift the current stable-loop main-stage reveal from page 17 to page 18.
- Shift the current precompile placeholder from page 18 to page 19.

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "placeholder explanation pages|stable loop"`

Expected: the focused page-loop composition tests pass.

### Task 4: Rewrite the formal page scripts for docs parity

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/16-第十六页-正式动画.md`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/17-第十七页-正式动画.md`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/18-第十八页-正式动画.md`
- Create: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/19-第十九页-正式动画.md`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs/剧本/README.md`

**Step 1: Write the doc expectations**

- Mirror the new semantics from the storyboard in the formal page scripts.
- Make sure page 16 and page 17 explain:
  - historical vs current `scl.csv`
  - what `stable` means
  - the “one old hash expands to two stable keys, then rebuilds to two new hashes” example

**Step 2: Update the docs**

- Keep the format aligned with the existing formal page scripts.
- Keep the tone diagram-first, not card-heavy.

**Step 3: Sanity-check references**

Run: `rg -n "第十九页|page_19|expand / build 在做什么|预编译怎么发生" /Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/Docs /Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src`

Expected: no stale references that still describe the old three-page structure.

### Task 5: Final verification

**Files:**
- Verify only

**Step 1: Run focused tests**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx -t "placeholder explanation pages|stable loop|placeholder-expanded loop chapter"`

Expected: all focused tests pass.

**Step 2: Run broader timeline/storyboard coverage**

Run: `npm test -- --run src/remotion/sceneTimeline.test.ts src/storyboard/pso-storyboard.test.ts`

Expected: all tests pass.

**Step 3: Run build**

Run: `npm run build`

Expected: successful Remotion/Vite build.

**Step 4: Commit**

```bash
git add Docs/plans/2026-04-13-slideapp-expand-build-page-split-implementation.md \
  Docs/剧本/16-第十六页-正式动画.md \
  Docs/剧本/17-第十七页-正式动画.md \
  Docs/剧本/18-第十八页-正式动画.md \
  Docs/剧本/19-第十九页-正式动画.md \
  Docs/剧本/README.md \
  SlideApp/src/storyboard-data/pso-workbench-types.ts \
  SlideApp/src/storyboard-data/pso-storyboard.ts \
  SlideApp/src/storyboard/pso-storyboard.test.ts \
  SlideApp/src/remotion/embed.ts \
  SlideApp/src/remotion/sceneTimeline.ts \
  SlideApp/src/remotion/sceneTimeline.test.ts \
  SlideApp/src/remotion/pages/page-layout-constants.ts \
  SlideApp/src/remotion/pages/Page10Scene.tsx \
  SlideApp/src/remotion/Composition.test.tsx
git commit -m "feat: split expand and build pages"
```
