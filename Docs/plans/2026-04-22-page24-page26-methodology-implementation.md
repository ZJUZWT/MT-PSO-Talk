# Page24 Page26 Methodology Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rebuild `page_24` and `page_26` into lighter methodology pages that explain `减体积 / 虚拟化 / 分集合 / 并行化` with less nested geometry and clearer scan order.

**Architecture:** Update storyboard copy first so the intended method language is explicit. Then reshape the Remotion page components to reduce node count and reflect the new logic. Mirror the simplified structure in the formal review registry, refresh the page scripts, and rerun the required harness audits for the late-tail section.

**Tech Stack:** React/Remotion, storyboard metadata, formal geometry review registry, Vitest, markdown page scripts, slide-geometry harness audits

---

### Task 1: Freeze the approved methodology wording

**Files:**
- Create: `docs/plans/2026-04-22-page24-page26-methodology-design.md`
- Create: `docs/plans/2026-04-22-page24-page26-methodology-implementation.md`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Test: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Update page24/page26 storyboard metadata**

- Rename and rewrite the affected steps around:
  - `资源过重：减体积 / 虚拟化`
  - `编译过慢：分集合 / 并行化`
- Ensure manuscript text contains the approved methodology terms and familiar analogies.

**Step 2: Update storyboard tests**

- Replace old expectations that focused on `LRU + mmap` as the title-level message or on queue/worker details as page26’s primary story.

### Task 2: Simplify page24 geometry around methodology instead of implementation nesting

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Test: `SlideApp/src/remotion/Composition.test.tsx`
- Test: `SlideApp/src/review/formalPageReviewRegistry.test.ts`

**Step 1: Keep the left compression evidence but restyle it as the `减体积` half**

- Preserve the three algorithm rows and platform pills.
- Add methodology header/corner-tag language.
- Avoid adding new nested containers.

**Step 2: Replace the old right-side strategy skeleton with a lighter `虚拟化` card**

- Remove the structure strip.
- Remove the lane shell.
- Remove the bottom method cards.
- Keep only the core residency / external-IO relationship plus two arrows and short labels.

**Step 3: Mirror the simplified right-side structure in the formal review registry**

- Update the page24 formal sketch so it reviews the new dominant nodes instead of the removed helper capsules.

### Task 3: Simplify page26 around `分集合` vs `并行化`

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Test: `SlideApp/src/remotion/Composition.test.tsx`
- Test: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`

**Step 1: Keep the left event chain and dual-mask comparison as the `分集合` half**

- Preserve:
  - event strip
  - `Game UsageMask = A`
  - `Compile UsageMask = A + B`
- Compress the explanatory note into a lighter summary strip.

**Step 2: Replace the old queue/worker topology with a lighter `并行化` methodology card**

- Remove the queue / worker / throughput graph.
- Replace it with:
  - the `并行化` title
  - condition text
  - `SIMD / Thread / GPU` hierarchy
  - a short PSO mapping note

**Step 3: Update tests and score thresholds if geometry meaningfully changes**

- Keep the page26 geometry above the existing late-tail mechanical floor.

### Task 4: Refresh formal page scripts and page-local contracts

**Files:**
- Modify: `Docs/剧本/24-第二十四页-正式动画.md`
- Modify: `Docs/剧本/26-第二十六页-正式动画.md`

**Step 1: Rewrite the page goals and change tables**

- Record the methodology framing and removed nested structures.

**Step 2: Update node / edge review tables**

- Remove obsolete helper nodes from the written contract.
- Add the new dominant methodology nodes and summaries.

**Step 3: Update transition/timing sections only where content description changed**

- Keep the frame anchors unless the timing audits force retiming.

### Task 5: Verify the rebuilt pages and close the workflow gate

**Files:**
- Modify if needed: `Docs/剧本/00-剧本与页面对齐审计.md`

**Step 1: Run focused tests**

Run:

```bash
npm --silent --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/remotion/Composition.test.tsx src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts
```

**Step 2: Run harness audits**

Run:

```bash
python3 scripts/slide-geometry-harness/audit_transition_timings.py
python3 scripts/slide-geometry-harness/audit_storyboard_sync.py
npm --silent --prefix SlideApp run review:mechanical -- --from page_19
```

**Step 3: Close workflow gate**

Run:

```bash
python3 scripts/slide-geometry-harness/workflow_gate.py stop --state-path .git/slide-geometry-harness-workflow-state.json
```
