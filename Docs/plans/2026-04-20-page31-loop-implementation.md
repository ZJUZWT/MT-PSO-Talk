# Page 31 Loop Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 把 `page_31` 改成完整 harness 回环页，并顺手修掉 `page_19` 的 formal route blocker。

**Architecture:** `page_31` 的 Remotion 正式页、storyboard 文案、formal review sketch 和剧本文档需要同步改；`page_19` 的 formal sketch 则通过一个更明确的几何测试锁定 `vk-to-disk` 锚点问题，再做最小坐标修复。

**Tech Stack:** React + Remotion + formal geometry harness + Vitest + repo audit scripts

---

### Task 1: Lock page_19 blocker with tests

**Files:**
- Modify: `SlideApp/src/review/page19VerticalRelief.test.ts`
- Modify: `SlideApp/src/harness/slide-geometry/contracts/page19-r1.ts`

**Step 1: Strengthen the failing test**

- Assert `nodePierceCount === 0`
- Assert `badEndpointCount === 0` for `vk-to-disk`
- Assert the `vk-to-disk` target anchor lands on `disk-vk` left edge instead of interior

**Step 2: Run test to verify RED**

Run: `npm --silent --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts`

Expected: FAIL because the current `vk-to-disk` endpoint still pierces `disk-vk`

**Step 3: Apply minimal fix**

- Move `vk-to-disk.to.x` from the stale interior point to the actual `disk-vk` left boundary
- Keep the route horizontal and centered vertically

**Step 4: Run test to verify GREEN**

Run: `npm --silent --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts`

Expected: PASS

### Task 2: Rebuild page_31 as a full loop page

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `SlideApp/src/components/NotesPanel.test.tsx`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`

**Step 1: Update tests/doc expectations first**

- Adjust `NotesPanel` and storyboard assertions so page31 must mention:
  - `Agent`
  - `hook`
  - `workflow_gate.py`
  - `前台 Edge`
  - `geometryReviewArtifact`
  - `geometryMetrics`
  - `geometryScorePolicy`

**Step 2: Run targeted tests to verify RED**

Run: `npm --silent --prefix SlideApp test -- src/components/NotesPanel.test.tsx src/storyboard/pso-storyboard.test.ts src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: FAIL on old page31 content / geometry surface

**Step 3: Implement page31 loop layout**

- Replace the old left “评分链路卡” with explicit loop nodes and arrows
- Keep a narrower right-side card for hook/gate/formula summaries
- Preserve existing late-tail visual language and footer rhythm

**Step 4: Update formal page31 review surface**

- Align formal node boxes with the new Remotion geometry
- Keep `overlap/crossing/pierce/textOverflow` clean

**Step 5: Run targeted tests to verify GREEN**

Run: `npm --silent --prefix SlideApp test -- src/components/NotesPanel.test.tsx src/storyboard/pso-storyboard.test.ts src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

Expected: PASS

### Task 3: Sync page31 script + verify full gate

**Files:**
- Modify: `Docs/剧本/31-第三十一页-正式动画.md`
- Modify: `Docs/剧本/00-剧本与页面对齐审计.md`

**Step 1: Refresh page31 script**

- Rewrite page goal / inventory / review wording to match the new loop
- Record that page31 now visualizes the actual automation loop:
  `Agent 修改 -> 前台 Edge 真实取数 -> artifact -> metrics -> policy -> 回写修改`

**Step 2: Verify implementation**

Run:
- `npm --silent --prefix SlideApp run build`
- `npm --silent --prefix SlideApp test -- src/review/page19VerticalRelief.test.ts src/components/NotesPanel.test.tsx src/storyboard/pso-storyboard.test.ts src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`
- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
- `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

Expected:
- build passes
- targeted tests pass
- audits pass
- mechanical review no longer reports page19 blocker
