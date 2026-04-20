# Page 19+ Real Browser Layout Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Give `page_19` through `page_33` real front-Edge text measurement coverage, then polish each page layout using that real data.

**Architecture:** Extend the late-tail page components with geometry probe hooks that map formal review node ids onto the actual Remotion page DOM, then iterate page-by-page with the existing review harness. Real browser probe data overrides text-fit metrics; formal sketches remain the source for non-text geometry structure.

**Tech Stack:** React, Remotion SVG pages, Vitest, existing slide geometry harness, AppleScript-backed Edge probe

---

### Task 1: Document the approved approach

**Files:**
- Create: `docs/plans/2026-04-20-page19plus-real-browser-layout-design.md`
- Create: `docs/plans/2026-04-20-page19plus-real-browser-layout-implementation.md`

**Step 1: Record the user-approved route**

Write the approved “real probe first, then page-by-page polish” workflow into the design doc.

**Step 2: Save the implementation plan**

Write the execution order and verification gates into this plan file.

### Task 2: Expand reusable geometry probe primitives

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/primitives/diagramPrimitives.tsx`

**Step 1: Add hook support to shared components**

Add optional geometry props to the shared late-tail building blocks so they can expose:

- node id
- node label
- measurable box
- measurable text

**Step 2: Verify no existing late-tail pages regress**

Run the targeted tests and build.

### Task 3: Add real probe coverage to page_19

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/harness/slide-geometry/contracts/page19-r1.ts`

**Step 1: Hook the page_19 formal nodes onto the real DOM**

Cover the major stable / UE / memory / disk blocks and any text-bearing nodes that the probe must measure.

**Step 2: Probe the front Edge page**

Run the front-browser probe and confirm `page_19` no longer returns `nodes: []`.

### Task 4: Fill the remaining page_24~33 real probe gaps

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`

**Step 1: Map every missing formal review node to a real page node**

Focus on:

- `page_24` table and link
- `page_25` left-card / memory / disk
- `page_26` left-card / masks / compile banner
- `page_27` left-card / queue / workers / done
- `page_28` images / right-card / state boxes
- `page_29` attribute tokens
- `page_30` images / tokens
- `page_31` left-card / chain / right-card / gates
- `page_32` left / right / game cards
- `page_33` quote / footer

**Step 2: Align formal review labels where needed**

If a formal review node still reflects old copy or old structure, update it to match the real page.

### Task 5: Re-measure each page with real front-Edge data

**Files:**
- Modify if needed: `SlideApp/src/review/frontBrowserGeometryTextProbe.ts`
- Modify if needed: `scripts/slide-geometry-harness/probe_front_browser_geometry_text.py`

**Step 1: Iterate page-by-page**

For each page from `page_19` to `page_33`, switch the front Edge tab, capture probe output, and note the concrete overflow / padding / font-size issues.

**Step 2: Build the fix list**

Use the real data, not guesswork, to decide what to fix on each page.

### Task 6: Polish pages one-by-one

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `Docs/剧本/19-第十九页-正式动画.md`
- Modify: `Docs/剧本/21-第二十一页-正式动画.md`
- Modify: `Docs/剧本/22-第二十二页-正式动画.md`
- Modify more page docs as needed under `Docs/剧本/`

**Step 1: Fix the current page**

Adjust spacing, card sizes, typography, alignment, and density based on the measured problem.

**Step 2: Re-probe the current page**

Confirm the real Edge measurements improved.

**Step 3: Update the page script**

Refresh the page markdown with the new facts and review outcome.

### Task 7: Lock in regression coverage

**Files:**
- Modify: `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.test.ts`
- Modify: `SlideApp/src/review/page19PlusReview.test.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Add or update tests for real probe wiring**

Make sure the late-tail pages keep their geometry node mapping.

**Step 2: Update page copy expectations where pages changed**

Keep the explicit page rendering tests aligned with the new copy.

### Task 8: Run the required verification sweep

**Files:**
- No code changes required unless verification fails

**Step 1: Run targeted build and tests**

- `npm --silent --prefix SlideApp run build`
- `npm --silent --prefix SlideApp test -- src/harness/slide-geometry/review/geometryReviewArtifact.test.ts src/review/page19PlusReview.test.ts src/review/formalPageReviewRegistry.test.ts`

**Step 2: Run required harness audits**

- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
- `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

**Step 3: Run real front-browser mechanical checks**

At minimum, rerun the pages that had real overflow or the lowest layout scores using `--front-browser-text-probe`.
