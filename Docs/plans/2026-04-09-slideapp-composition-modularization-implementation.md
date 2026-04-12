# SlideApp Composition Modularization Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refactor the Remotion composition into shared geometry/primitives plus one render module per page, while preserving the current rendered behavior and test coverage.

**Architecture:** Keep one global scene-model computation layer for timeline math and cross-page projections, but move page rendering, primitives, and layout constants out of the monolithic `Composition.tsx`. `Composition.tsx` becomes a thin orchestrator over the computed model and page renderer modules.

**Tech Stack:** React, TypeScript, Remotion SVG composition, Vitest.

---

### Task 1: Add regression coverage that protects the new module boundary

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add tests that assert:

- `SceneSvg` still renders after the modular split
- the page 08 proof card, PSO box, and hash arrows still appear at frame `306`
- the page 09 shared library still appears at frame `342`

**Step 2: Run test to verify it fails**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL once the assertions target modules that do not exist yet.

**Step 3: Write minimal implementation support**

Update imports and test selectors as needed once the split starts landing.

**Step 4: Run test to verify it passes**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 2: Extract shared geometry and primitive modules

**Files:**
- Create: `SlideApp/src/remotion/primitives/diagramTypes.ts`
- Create: `SlideApp/src/remotion/primitives/diagramPrimitives.tsx`
- Create: `SlideApp/src/remotion/geometry/geometry.ts`
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write the failing test**

Run the current composition test after removing one local helper or primitive from `Composition.tsx`.

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL with missing import / missing symbol.

**Step 2: Write minimal implementation**

Move:

- shared types such as `Box`, `RgbaColor`
- geometry helpers such as `mix`, `mixBox`, `polylinePath`, `scalePointAround`
- reusable diagram components such as `StageBox`, `StrokeArrow`, `ArrowLabelPill`, `CalloutBadge`

into the new shared modules and re-import them from `Composition.tsx`.

**Step 3: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 3: Extract page layout constants into a dedicated module

**Files:**
- Create: `SlideApp/src/remotion/pages/page-layout-constants.ts`
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write the failing test**

Temporarily remove one page box constant from `Composition.tsx` and run:

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL because page layout constants are unresolved.

**Step 2: Write minimal implementation**

Move all `PAGE*_..._BOX` constants and frame constants into the new layout module, then import them back into the scene model / composition entrypoint.

**Step 3: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 4: Extract scene-model computation out of `Composition.tsx`

**Files:**
- Create: `SlideApp/src/remotion/model/scene-model-types.ts`
- Create: `SlideApp/src/remotion/model/computeSceneModel.ts`
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write the failing test**

Replace direct local variable usage in `Composition.tsx` with a temporary missing `computeSceneModel(...)` import.

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL.

**Step 2: Write minimal implementation**

Move the current large block of progress/geometry/value derivation into `computeSceneModel.ts`, and define a `SceneModel` type containing the data needed by page renderers.

Do not over-normalize. It is acceptable for `SceneModel` to be large in this first pass as long as ownership is clearer than today.

**Step 3: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 5: Split render output into one file per page

**Files:**
- Create: `SlideApp/src/remotion/pages/Page01Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page02Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page03Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page04Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page05Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page06Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page07Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page08Scene.tsx`
- Create: `SlideApp/src/remotion/pages/Page09Scene.tsx`
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write the failing test**

Update `Composition.tsx` to call missing page renderers.

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL until the page files exist.

**Step 2: Write minimal implementation**

Move each page's render branch into its own module. Each module should export a function or component that accepts `SceneModel` and returns the owned JSX fragment.

Suggested ownership:

- Page 01: formula base
- Page 02: GPU/input/output morph additions
- Page 03: OpenGL upper-band workflow
- Page 04: Vulkan PSO workflow
- Page 05: UE asset cook bridge
- Page 06: ownership / selector stage
- Page 07: InlineCode lookup expansion
- Page 08: PSO hash proof / external material proof
- Page 09: SharedCode library

**Step 3: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 6: Make `Composition.tsx` a thin orchestrator

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write the failing test**

Run:

`npm test -- src/remotion/Composition.test.tsx`

Expected: FAIL during the intermediate state while `Composition.tsx` still duplicates moved logic.

**Step 2: Write minimal implementation**

Reduce `Composition.tsx` to:

- imports
- `SceneSvgProps`
- `SceneSvg`
- `MyComposition`

Inside `SceneSvg`, compute the model and render:

- base wrappers
- page renderer sequence

**Step 3: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 7: Full verification

**Files:**
- Modify as needed based on failures

**Step 1: Run full tests**

Run: `npm test`
Expected: PASS.

**Step 2: Run production build**

Run: `npm run build`
Expected: PASS.

**Step 3: Commit**

```bash
git add Docs/plans/2026-04-09-slideapp-composition-modularization-design.md Docs/plans/2026-04-09-slideapp-composition-modularization-implementation.md SlideApp/src/remotion SlideApp/src/components/RemotionStage.tsx SlideApp/src/App.tsx SlideApp/src/App.test.tsx SlideApp/src/app.css
git commit -m "refactor: modularize slide composition scene"
```
