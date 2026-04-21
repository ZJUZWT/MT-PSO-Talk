# Late-Tail Rebalance Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rebalance page 19, page 24, and page 26 so late-tail content is grouped by problem type instead of being over-compressed into one layout.

**Architecture:** Update the Remotion page components first, then synchronize storyboard copy, tests, and formal page scripts so the geometry harness and doc audits describe the new host/retained-page roles correctly. Verification must use the repo's timing audit, storyboard sync audit, and formal mechanical review after targeted tests and real-browser checks.

**Tech Stack:** React, Remotion, Vitest, Slide geometry harness scripts, formal page markdown under `Docs/剧本/`

---

### Task 1: Rebalance page 19 geometry

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Update the page-19 failing expectations**

- Replace assertions that assume one `binary-summary` strip with assertions for three note nodes under the main diagram.

**Step 2: Implement the new page-19 note layout**

- Remove `PAGE19_BINARY_SUMMARY_BOX`.
- Add three compact note boxes under the main diagram.
- Keep `Metal Binary Archive 2 ?` as a separate badge.

**Step 3: Re-run targeted page-19 composition tests**

- Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 19"`

### Task 2: Rebalance page 24 into package/memory dual groups

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Update the page-24 failing expectations**

- Replace three-row assertions with two-group assertions:
  - compression host + data host
  - storage host + carrier host

**Step 2: Implement the new page-24 layout**

- Rewrite `Page24StrategyPage`.
- Remove the parallel row from page 24.
- Keep compression metrics and storage/carry wording.

**Step 3: Re-run targeted page-24 composition tests**

- Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 24"`

### Task 3: Rebalance page 26 into two compile-optimization paths

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Add/update page-26 tests**

- Assert that page 26 contains both:
  - `Game UsageMask / Compile UsageMask`
  - `并行 / 任务独立 / 纯 CPU 计算`
- Assert the two paths are presented as separate modules, not one merged sentence.

**Step 2: Implement the new page-26 layout**

- Keep the event-first left/top flow.
- Add a parallel optimization card/group as a second optimization path.
- Update footer/copy so the message is “减少集合 + 提升吞吐”。

**Step 3: Re-run targeted page-26 composition tests**

- Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 26"`

### Task 4: Sync storyboard and formal page scripts

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `Docs/剧本/19-第十九页-正式动画.md`
- Modify: `Docs/剧本/24-第二十四页-正式动画.md`
- Modify: `Docs/剧本/26-第二十六页-正式动画.md`
- Modify: `Docs/剧本/27-第二十七页-正式动画.md`

**Step 1: Update storyboard copy**

- Move page-24 narrative to package/memory grouping.
- Move page-27 retained-page note to say its core info now lives on page 26.
- Update page-26 manuscript to describe two parallel optimization levers.

**Step 2: Update formal page scripts**

- Rewrite changed node/edge inventories and page goals.
- Keep transition sections unless frame anchors actually change.

**Step 3: Re-run storyboard tests**

- Run: `npm --silent --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`

### Task 5: Run harness verification and report actual state

**Files:**
- Modify if needed based on audit output: changed page scripts or timeline files

**Step 1: Capture/inspect real-browser result if text tightness changes**

- Use the existing front-browser / browser-api path already established in this repo.

**Step 2: Run required repo verification**

- Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 19|page 24|page 26"`
- Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
- Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_19 --front-browser-text-probe`

**Step 3: Only then summarize**

- Report exact pass/fail state.
- Call out any remaining comfort-floor issues separately from hard blockers.
