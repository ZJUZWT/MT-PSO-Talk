# Page 19 Vertical Relief Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Adjust `page_19` with a vertical-relief layout pass that preserves the current reading structure while improving the page's harness geometry facts.

**Architecture:** Keep the runtime `page_19` composition and the formal sketch contract in sync. First lock the intended geometry direction with focused tests, then update only the `page_19` constants, edge endpoints, and formal sketch boxes, and finally rerun the page review artifacts and refresh the `page_19` ledger facts.

**Tech Stack:** TypeScript, React, Remotion, Vitest, slide-geometry review artifact builder, Markdown

---

### Task 1: Lock the intended page_19 geometry direction in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Create or modify: `SlideApp/src/review/page19VerticalRelief.test.ts`

**Step 1: Write the failing tests**

- Add one focused test for the formal `page_19` sketch that expects:
  - `nodePierceCount = 0`
  - `minRenderedFontPx` improved from the current 15px floor
  - `minInternalPadding` improved from the current 4px floor
- Add one focused runtime test that keeps the current semantic structure visible:
  - `stable.` / `upipelinecache`
  - `UE PSO`
  - `GPU`
  - `内存中 PSO`
  - `硬盘中的 PSO`

**Step 2: Run the focused tests and confirm failure**

Run: `npm --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts src/remotion/Composition.test.tsx`

Expected: FAIL because the current page still has the old cramped geometry.

### Task 2: Update the page_19 formal sketch contract

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/contracts/page19-r1.ts`
- Test: `SlideApp/src/review/page19VerticalRelief.test.ts`

**Step 1: Expand the vertical budget in the formal sketch**

- Increase `UE`, `memory`, and `disk` group height.
- Increase the 3 child row heights where padding is currently too tight.
- Keep the horizontal reading order unchanged.

**Step 2: Fix the GPU vertical-edge anchors in the sketch**

- Land the upper and lower GPU links on the GPU node boundary instead of inside the node body.

**Step 3: Run the sketch-focused test**

Run: `npm --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts`

Expected: PASS.

### Task 3: Update the runtime page_19 layout to match the contract

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Test: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Apply the vertical-relief box changes**

- Update only the `PAGE19_*` constants used by `page_19`.
- Keep the five-part horizontal reading structure.

**Step 2: Fix the GPU vertical routes**

- Move the upper/lower vertical arrow endpoints to the GPU boundary.

**Step 3: Keep text hierarchy and labels intact**

- Preserve the currently approved copy and runtime structure.

**Step 4: Run the runtime-focused test**

Run: `npm --prefix SlideApp test -- src/remotion/Composition.test.tsx`

Expected: PASS for the page_19 structure assertions.

### Task 4: Regenerate page_19 review facts

**Files:**
- Generated: `ignore/slide-review/page_19_plus/pages/page_19.geometry.json`
- Generated: `ignore/slide-review/page_19_plus/pages/page_19.geometry.md`
- Modify: `Docs/剧本/19-第十九页-正式动画.md`

**Step 1: Run the review command**

Run: `npm --prefix SlideApp run review:slides -- --from page_19`

Expected: PASS and fresh artifacts under `ignore/slide-review/page_19_plus/`.

**Step 2: Update the page 19 ledger facts**

- Refresh the geometry score/fact section to match the new artifact.
- Only update page 19 facts; do not rewrite older pages.

### Task 5: Verify the change set

**Files:**
- Modified files from Tasks 1-4

**Step 1: Run focused tests**

Run: `npm --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts src/remotion/Composition.test.tsx`

Expected: PASS.

**Step 2: Run build**

Run: `npm --prefix SlideApp run build`

Expected: PASS.

**Step 3: Inspect git status**

Run: `git -C SlideApp status --short`

Expected: only page_19-related runtime, formal sketch, review, and ledger updates plus existing unrelated user edits.
