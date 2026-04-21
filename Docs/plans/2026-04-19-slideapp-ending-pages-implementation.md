# SlideApp Ending Pages Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add two new ending pages after the current strategy chapter: a linked reading/recommendation page and a final Zhuangzi quote closing page.

**Architecture:** Extend the canonical storyboard/timeline from `page_27` to `page_29`, render both ending pages inside the existing `Page10Scene` placeholder tail, then document the new pages and their transition timing facts in `Docs/剧本/`.

**Tech Stack:** TypeScript, Remotion, Vitest, slide-geometry-harness timing scripts, Markdown ledger docs

---

### Task 1: Lock the new ending pages in tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.test.ts`

**Step 1: Write the failing test**

- Extend the expected canonical step order to include `page_28` and `page_29`.
- Assert `page_28` contains the reading/recommendation content and `page_29` contains the `逍遥游` closing quote.
- Extend timeline tests so `REMOTION_STEP_SEQUENCE` and frame anchors include the two new steps.

**Step 2: Run test to verify it fails**

Run: `npm test -- SlideApp/src/storyboard/pso-storyboard.test.ts SlideApp/src/remotion/sceneTimeline.test.ts`

Expected: FAIL because `page_28` / `page_29` do not exist yet.

**Step 3: Write minimal implementation**

- No production implementation in this task.

**Step 4: Run test to verify it fails for the expected reason**

Run: `npm test -- SlideApp/src/storyboard/pso-storyboard.test.ts SlideApp/src/remotion/sceneTimeline.test.ts`

Expected: FAIL with missing step/frame assertions.

### Task 2: Extend storyboard and Remotion timeline

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`

**Step 1: Add new step ids**

- Extend `StoryStepId` with `page_28` and `page_29`.

**Step 2: Add storyboard metadata**

- Append `page_28` and `page_29` to the strategy-tail session.
- Add final-page copy, related links, and manuscript text.

**Step 3: Add frame anchors**

- Append new frame anchors in `embed.ts`.
- Extend the Remotion step sequence in `sceneTimeline.ts`.

**Step 4: Re-run tests**

Run: `npm test -- SlideApp/src/storyboard/pso-storyboard.test.ts SlideApp/src/remotion/sceneTimeline.test.ts`

Expected: PASS.

### Task 3: Render the two ending pages

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Add page-specific placeholder components**

- Create a recommendation page layout with two columns and visible link text.
- Create a final quote page layout with large centered quote and source line.

**Step 2: Thread the new reveal windows**

- Add reveal/focus calculations for `page_28` and `page_29`.
- Mount the new components after `page_27`.

**Step 3: Run focused rendering tests**

Run: `npm test -- SlideApp/src/remotion/Composition.test.tsx`

Expected: PASS.

### Task 4: Document the new pages and timing facts

**Files:**
- Create: `Docs/剧本/28-第二十八页-正式动画.md`
- Create: `Docs/剧本/29-第二十九页-正式动画.md`
- Modify: `Docs/剧本/README.md`
- Modify: `Docs/剧本/00-剧本与页面对齐审计.md` (generated)

**Step 1: Add new page ledgers**

- Use the page template structure.
- Record page goals, node/edge inventory, and ending-page copy.

**Step 2: Add transition timing facts**

- Create workload JSON for `page_27 -> page_28` and `page_28 -> page_29`.
- Run the timing probe script and paste the markdown output into the corresponding page ledgers.

**Step 3: Run the repository audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: the audit output updates `Docs/剧本/00-剧本与页面对齐审计.md` with the new pages present.
