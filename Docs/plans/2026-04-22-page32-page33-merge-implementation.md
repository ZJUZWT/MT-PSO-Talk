# Page 32 / 33 Merge Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Merge the old `page_32` and `page_33` into one final epilogue page and delete `page_33` as a standalone step.

**Architecture:** Update the storyboard/timeline layer first so only one final step remains, then rebuild the late-tail page render so the quote occupies the upper half and the recommendations occupy two lower groups. Finally, sync review geometry artifacts, docs, and tests so the harness sees one final page instead of two.

**Tech Stack:** TypeScript, React, Remotion, Vitest, slide-geometry harness audits

---

### Task 1: Remove `page_33` from step metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/embed.ts`

### Task 2: Rebuild final-page content on `page_32`

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

### Task 3: Sync review geometry and page scripts

**Files:**
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `Docs/剧本/32-第三十二页-正式动画.md`
- Delete: `Docs/剧本/33-第三十三页-正式动画.md`

### Task 4: Update tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/state/useWorkbenchState.test.ts`
- Modify: `SlideApp/src/components/NotesPanel.test.tsx`
- Modify: `SlideApp/src/remotion/sceneTimeline.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/review/page19PlusReview.test.ts`

### Task 5: Run verification

**Commands:**
- `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`
- `npm --prefix SlideApp test -- src/remotion/sceneTimeline.test.ts`
- `npm --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 32|page 33"`
- `npm --prefix SlideApp test -- src/remotion/pages/lateTailGeometry.test.ts`
- `npm --prefix SlideApp test -- src/review/formalPageReviewRegistry.test.ts`
- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
- `npm --silent --prefix SlideApp run review:mechanical -- --from page_10`
