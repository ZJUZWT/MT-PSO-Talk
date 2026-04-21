# Page24 Density Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Densify `page_24` so the current whitespace is roughly halved without changing the slide’s left/right meaning or introducing informal text.

**Architecture:** Adjust the page24 card geometry first, then use the newly available space to thicken the compression matrix and the UE memory strategy structure. Mirror those coordinates in the formal review registry and refresh the script/storyboard descriptions so harness artifacts stay in sync.

**Tech Stack:** React/Remotion, formal geometry review registry, Jest, Python harness audits, markdown slide script docs

---

### Task 1: Record the approved direction

**Files:**
- Create: `docs/plans/2026-04-21-page24-density-design.md`
- Create: `docs/plans/2026-04-21-page24-density-implementation.md`

**Step 1: Save the approved design**

Write the approved `A` direction plus the current and target whitespace budget.

**Step 2: Save the implementation plan**

Write the file list, geometry intent, and required verification commands.

### Task 2: Tighten page24 geometry and structure

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Test: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Update page24 card coordinates**

Move and enlarge the left/right cards so outer whitespace is substantially reduced.

**Step 2: Rebalance the left compression rows**

Use the added height/width to make each row denser and more structured while preserving the existing data.

**Step 3: Rebalance the right memory strategy layout**

Use the added area to separate residency, swap/reload, and mapping/carrier structure more clearly.

**Step 4: Mirror geometry in the formal review registry**

Keep the review sketch aligned with the rendered page24 geometry.

**Step 5: Update test expectations if coordinates or structural surfaces change**

Preserve the page24 pill-coverage regression checks.

### Task 3: Refresh docs and rerun harness checks

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `Docs/剧本/24-第二十四页-正式动画.md`

**Step 1: Update storyboard copy**

Describe the denser formal matrix and richer right-side structure.

**Step 2: Update the page24 script**

Record the new nodes, moved nodes, and revised review/timing facts.

**Step 3: Run required verification**

Run:

```bash
npm --prefix SlideApp test -- lateTailGeometry formalPageReviewRegistry pso-storyboard
python3 scripts/slide-geometry-harness/audit_transition_timings.py --output-dir generated/transition-timing-audit
python3 scripts/slide-geometry-harness/audit_storyboard_sync.py
```

**Step 4: If needed, rerun targeted page24 checks**

Run:

```bash
npm --prefix SlideApp test -- Composition -t "page 24"
```
