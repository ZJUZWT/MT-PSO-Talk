# Formal Page Review Scoring Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make formal Remotion pages the default mechanical review target, keep sketch review as fallback, and surface compact score feedback through the Codex workflow gate without duplicating harness formulas.

**Architecture:** Introduce a repo-local formal review registry that exposes formal-page geometry as harness-compatible review surfaces. Route App review mode and batch review through a single `formal-first -> sketch-fallback` resolver, then add a thin review-summary CLI so the Python workflow gate can include computed geometry results beside timing audits.

**Tech Stack:** TypeScript, React, Vitest, Python 3.11, `vite-node`, existing slide geometry harness review modules.

---

### Task 1: Add failing tests for formal-first review resolution

**Files:**
- Create: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/App.test.tsx`
- Modify: `SlideApp/src/review/page19PlusReview.test.ts`

**Step 1: Write the failing tests**

Cover:
- formal review definitions exist for the late-tail formal pages and build geometry artifacts through the existing score chain
- story review mode prefers formal review on a formal page instead of dropping to manual-only HUD controls
- `runPage19PlusReview(...)` uses formal review for text pages like `page_21` and `page_22` rather than returning `missing_sketch`

**Step 2: Run tests to verify they fail**

Run:
```bash
npm --prefix SlideApp test -- --runInBand src/review/formalPageReviewRegistry.test.ts src/App.test.tsx src/review/page19PlusReview.test.ts
```

Expected:
- missing module / export failures, or assertions showing story review still falls back to manual-only mode

### Task 2: Implement the formal review registry and shared resolver

**Files:**
- Create: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Create: `SlideApp/src/review/geometryReviewSurface.ts`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`

**Step 1: Write minimal implementation**

Implement:
- formal-page review surfaces for `page_19`, `page_21`, `page_22`, and `page_24` to `page_33`
- a shared resolver that returns:
  - explicit sketch review in sketch mode
  - otherwise formal review if available
  - otherwise legacy sketch fallback
- metadata describing whether the active review artifact came from `formal` or `sketch`

**Step 2: Run the targeted tests**

Run:
```bash
npm --prefix SlideApp test -- src/review/formalPageReviewRegistry.test.ts src/review/page19PlusReview.test.ts
```

Expected:
- PASS

### Task 3: Wire formal-first review into the app and batch review

**Files:**
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/components/ReviewHud.tsx`
- Modify: `SlideApp/src/review/page19PlusReview.ts`
- Modify: `SlideApp/src/review/geometryReviewSummary.ts`

**Step 1: Update runtime review wiring**

Implement:
- `App.tsx` builds the active review artifact from the shared resolver instead of sketch-only state
- `ReviewHud.tsx` renders the same fact-bound score UI for both formal and sketch review sources
- `page19PlusReview.ts` records the review source and summary for formal pages, with sketch only as fallback
- summary/markdown output keeps the required three-layer review fields

**Step 2: Run focused app tests**

Run:
```bash
npm --prefix SlideApp test -- src/App.test.tsx src/review/page19PlusReview.test.ts
```

Expected:
- PASS

### Task 4: Add a compact mechanical review summary CLI and hook integration

**Files:**
- Create: `SlideApp/scripts/reviewMechanicalSummary.ts`
- Modify: `SlideApp/package.json`
- Modify: `scripts/slide-geometry-harness/workflow_gate.py`
- Modify: `scripts/slide-geometry-harness/tests/test_workflow_gate.py`

**Step 1: Write failing hook tests**

Cover:
- full slide-harness work now requires the mechanical review summary command
- stop output blocks when the computed review summary reports blocker pages
- stop output includes a compact score payload that names the failing page(s) and the top fixes

**Step 2: Implement the thin integration**

Implement:
- a `review:mechanical` CLI that prints compact JSON from `runPage19PlusReview(...)`
- Python hook helpers that run this CLI, parse the JSON, and add score feedback to stop decisions
- gating based on existing review outputs only, especially `blockerOpen` / artifact verdicts, with no duplicate formulas

**Step 3: Run the hook tests**

Run:
```bash
python3 -m unittest discover -s scripts/slide-geometry-harness/tests -p 'test_*.py' -v
```

Expected:
- PASS

### Task 5: Run end-to-end verification

**Files:**
- Verify only

**Step 1: Run SlideApp targeted tests**

Run:
```bash
npm --prefix SlideApp test -- src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts src/review/page19PlusReview.test.ts src/App.test.tsx
```

Expected:
- PASS

**Step 2: Run workflow-gate tests**

Run:
```bash
python3 -m unittest discover -s scripts/slide-geometry-harness/tests -p 'test_*.py' -v
```

Expected:
- PASS

**Step 3: Run a SlideApp safety build**

Run:
```bash
npm --prefix SlideApp run build
```

Expected:
- PASS

**Step 4: Run the mandatory harness audits**

Run:
```bash
python3 scripts/slide-geometry-harness/audit_transition_timings.py
python3 scripts/slide-geometry-harness/audit_storyboard_sync.py
```

Expected:
- transition audit completes and reports the current timing verdict set
- storyboard sync audit completes and refreshes the doc audit artifact
