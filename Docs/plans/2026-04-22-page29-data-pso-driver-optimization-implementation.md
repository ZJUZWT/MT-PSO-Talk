# Page29 Data PSO Driver Optimization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add `page_29_data` as a new data evidence page between `page_29` and `page_30`, render the raw benchmark comparison table plus summary, and update docs/timing/tests so the new inserted step is fully integrated.

**Architecture:** Extend the late-tail step graph with a new inserted `_data` page. Reuse the existing late-tail placeholder shell and formal review patterns so the new page behaves like a first-class formal page instead of an ad-hoc overlay. Shift later timeline anchors forward to make room for the inserted transition without compressing downstream motion.

**Tech Stack:** Remotion React scenes, storyboard metadata, formal geometry review registry, Vitest, slide-geometry timing probe workloads, storyboard/doc sync audit.

---

### Task 1: Add the new step id and storyboard metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/state/useWorkbenchState.test.ts`

**Step 1: Write the failing tests**

- Add expectations for `page_29_data` in canonical step order and relevant session order.
- Add metadata assertions for the new title, notes, and manuscript highlights.

**Step 2: Run tests to verify they fail**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/state/useWorkbenchState.test.ts`

Expected: FAIL because `page_29_data` does not exist yet.

**Step 3: Write minimal storyboard implementation**

- Add `page_29_data` to the step id union and the late-tail session.
- Define its label, caption, key points, notes, and manuscript around the approved message.

**Step 4: Run tests to verify they pass**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/state/useWorkbenchState.test.ts`

Expected: PASS.

### Task 2: Insert the page into the Remotion timeline and render it

**Files:**
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/sceneTimeline.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing timeline/render tests**

- Add timeline assertions for `page_29_data`.
- Add page-render assertions for the new page title and selected raw-data values.

**Step 2: Run tests to verify they fail**

Run: `npm --prefix SlideApp test -- src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx`

Expected: FAIL because the new step is not yet in the timeline or scene render.

**Step 3: Write minimal implementation**

- Insert `page_29_data` between `page_29` and `page_30`.
- Shift later anchors forward by the inserted duration budget.
- Render a new late-tail page component with:
  - title
  - two platform cards
  - two rows (`10`, `5000`)
  - four metric columns per platform
  - summary strips

**Step 4: Run tests to verify they pass**

Run: `npm --prefix SlideApp test -- src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx`

Expected: PASS.

### Task 3: Register the new page in the formal review chain

**Files:**
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`

**Step 1: Write the failing review tests**

- Add `page_29_data` to the formal review registry expectations.
- Add a minimum geometry/mechanical score expectation for the new page.

**Step 2: Run tests to verify they fail**

Run: `npm --prefix SlideApp test -- src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: FAIL because the new formal page review surface does not exist yet.

**Step 3: Write minimal implementation**

- Register the new card/table/summary geometry nodes.
- Keep overlap / crossing / overflow metrics within hard gates.

**Step 4: Run tests to verify they pass**

Run: `npm --prefix SlideApp test -- src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: PASS.

### Task 4: Add the formal page script and transition workloads

**Files:**
- Create: `Docs/剧本/page_29_data-正式动画.md`
- Create: `Docs/剧本/workloads/page_29_to_page_29_data.json`
- Create: `Docs/剧本/workloads/page_29_data_to_page_30.json`
- Modify: `Docs/剧本/29-第二十九页-正式动画.md`
- Modify: `Docs/剧本/30-第三十页-正式动画.md`
- Modify: `Docs/剧本/README.md`

**Step 1: Write the page script and timing inputs**

- Record page goal, node inventory, edge inventory, and transition explanations.
- Update neighbor pages so they reference the new handoff instead of jumping straight through.

**Step 2: Run required audits**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: PASS / updated audit output with no missing-doc mismatch for `page_29_data`.

### Task 5: Verify the integrated late tail and close the workflow gate

**Files:**
- Modify if needed: `Docs/剧本/00-剧本与页面对齐审计.md`

**Step 1: Run focused late-tail tests**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/state/useWorkbenchState.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: PASS.

**Step 2: Run storyboard sync audit**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: PASS / updated audit markdown with the new page aligned.

**Step 3: Close workflow gate**

Run: `python3 scripts/slide-geometry-harness/workflow_gate.py stop --state-path .git/slide-geometry-harness-workflow-state.json`

Expected: no missing required checks for this turn.
