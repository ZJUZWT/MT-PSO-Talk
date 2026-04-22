# Page 31-33 Feedback Bridge Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rebuild the late-tail epilogue into three pages so `page_31` shows the live harness loop, `page_32` acts as a feedback-system bridge, and `page_33` restores the standalone final quote/recommendation ending.

**Architecture:** Lock the new three-page contract in tests first, then restore `page_33` across storyboard/timeline metadata, rebuild the Remotion late-tail renderers, and finally sync formal review surfaces, workloads, and page ledgers so the harness sees the same three-page structure.

**Tech Stack:** TypeScript, React, Remotion, Vitest, slide-geometry harness audits, Markdown, JSON

---

### Task 1: Lock the three-page epilogue contract in tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Modify: `SlideApp/src/state/useWorkbenchState.test.ts`
- Modify: `SlideApp/src/components/NotesPanel.test.tsx`

**Step 1: Write the failing assertions**

- Expect session 6 to contain `page_31`, `page_32`, and `page_33`.
- Expect `page_31` to render the live harness loop wording.
- Expect `page_32` to render the feedback-system bridge wording.
- Expect `page_33` to render the final quote plus recommendations.

**Step 2: Run focused tests and confirm failure**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx src/state/useWorkbenchState.test.ts src/components/NotesPanel.test.tsx`

Expected: FAIL because the current code still has only two epilogue pages and the old merged ending copy.

### Task 2: Restore `page_33` in canonical step metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`
- Modify: `scripts/slide-geometry-harness/workflow_gate.py`

**Step 1: Add `page_33` back to every canonical step sequence**

- Extend type unions, sessions, timeline order, and workflow-gate page order.

**Step 2: Retell the late-tail storyboard copy**

- Rewrite `page_31` metadata around the live harness loop.
- Add the new reflective `page_32`.
- Move the current ending copy down to `page_33`.

**Step 3: Retime the tail**

- Add a new `page_33` frame anchor and extend composition duration so the extra ending page does not get squeezed into the current budget.

### Task 3: Rebuild the Remotion late-tail renderers

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Rewrite `page_31` around the live harness loop**

- Replace the current internal geometry-chain emphasis with:
  - `Hook 进入`
  - `网页数据评分`
  - `网页图片评分`
  - `回执循环`
- Use the helper card to show the real implementation carriers and the `通过 / 继续` rule.

**Step 2: Add the new `page_32` bridge page**

- Render one reflective text-first page that connects:
  - feedback system
  - human learning
  - decreasing marginal gain of pure coding-only growth
  - future value of seemingly non-engineering knowledge

**Step 3: Move the current ending render to `page_33`**

- Reuse the existing quote + recommendation layout as the new final page.

**Step 4: Thread `page_33` into the reveal / focus chain**

- Add `LOOP_PAGE33_FRAME`, `page33PlaceholderReveal`, `page33PlaceholderVisible`, and the new fade-out suppression from `page_32` to `page_33`.

### Task 4: Sync formal review surfaces and harness tests

**Files:**
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/review/page19PlusReview.test.ts`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`

**Step 1: Rebuild formal page sketches for `page_31` and `page_32`**

- Match the new live harness loop structure on `page_31`.
- Add a text-first bridge layout for `page_32`.

**Step 2: Add the restored `page_33` review surface**

- Reuse the mature quote + recommendation geometry as the final formal review surface.

**Step 3: Update review expectations**

- Extend late-tail review scope to `page_33`.
- Adjust expected node ids and score thresholds to the new layouts.

### Task 5: Update page ledgers and transition workloads

**Files:**
- Modify: `Docs/剧本/31-第三十一页-正式动画.md`
- Modify: `Docs/剧本/32-第三十二页-正式动画.md`
- Add: `Docs/剧本/33-第三十三页-正式动画.md`
- Modify: `Docs/剧本/workloads/page_31_to_page_32.json`
- Modify: `Docs/剧本/workloads/page_32_to_page_33.json`

**Step 1: Rewrite the three page ledgers**

- Sync page goals, node inventories, changed elements, and transition descriptions with the new design.

**Step 2: Update workload JSON**

- `page_31 -> page_32` should describe the harness-to-feedback bridge.
- `page_32 -> page_33` should describe the bridge-to-final-ending handoff.

**Step 3: Generate timing review output and paste it back**

- Run `probe_transition_timeline.py` for both transitions and update the page markdown timing sections.

### Task 6: Run verification and audits

**Files:**
- Generated outputs under `Docs/剧本/`

**Step 1: Run focused tests**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx src/state/useWorkbenchState.test.ts src/components/NotesPanel.test.tsx src/review/formalPageReviewRegistry.test.ts src/review/page19PlusReview.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: PASS

**Step 2: Run transition and storyboard audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: PASS

**Step 3: Run the mechanical review summary from the new focus step**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_31`

Expected: PASS with no blocker pages for the late-tail epilogue.
