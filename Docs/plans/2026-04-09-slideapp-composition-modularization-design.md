# SlideApp Composition Modularization Design

**Assumption:** The user explicitly requested a one-shot refactor and explicitly waived further confirmation for this round. This design therefore chooses the recommended architecture directly and proceeds without additional approval checkpoints.

## Goal

Refactor `SlideApp` so the current monolithic Remotion composition no longer blocks iteration on layout, page intent communication, PPT readability tuning, or per-page debugging speed.

## Problems Being Solved

| Problem | Root Cause | Design Response |
| --- | --- | --- |
| Hard to make layout match the mental target | Layout constants, geometry derivation, and render logic are mixed in one file | Separate shared layout constants from render code and move page rendering into per-page files |
| Hard to communicate page intent | A page is currently "a region inside a giant timeline file" instead of an isolated scene module | Give each page its own render module with a clear ownership boundary |
| Hard to control PPT-safe typography | Font sizes are scattered as ad-hoc literals across the composition | Centralize common diagram tokens and keep page-local typography near the owning page |
| Iteration slows down over time | Editing one page requires reading global context for many pages and transitions | Split shared primitives, geometry helpers, and page renderers so most page changes stay local |

## Chosen Architecture

### 1. Keep a single scene model, but move it out of `Composition.tsx`

`SceneSvg` still needs one global timeline model because transitions span adjacent pages and share camera state. Instead of pretending the scene is fully independent per page, create a dedicated model layer:

- `src/remotion/model/computeSceneModel.ts`
- `src/remotion/model/scene-model-types.ts`

This layer owns:

- progress windows
- mixed boxes and centers
- shared timeline values
- projected/global overlay coordinates

This keeps the hard geometry together, but out of the page renderer files.

### 2. Split reusable SVG building blocks into shared modules

Create shared files for:

- `src/remotion/primitives/diagramPrimitives.tsx`
- `src/remotion/primitives/diagramTypes.ts`
- `src/remotion/geometry/geometry.ts`

These modules own:

- `StageBox`
- `StackedLabel`
- `TspanStackedLabel`
- `ArrowPath`
- `ArrowHead`
- `StrokeArrow`
- `ArrowLabelPill`
- `ApiBadge`
- `CalloutBadge`
- geometry helpers such as `mixBox`, `polylinePath`, `scalePointAround`

This reduces the mental noise in every page renderer and prevents future page files from re-embedding utility code.

### 3. Split page rendering into one file per page

Create:

- `src/remotion/pages/Page01Scene.tsx`
- `src/remotion/pages/Page02Scene.tsx`
- `src/remotion/pages/Page03Scene.tsx`
- `src/remotion/pages/Page04Scene.tsx`
- `src/remotion/pages/Page05Scene.tsx`
- `src/remotion/pages/Page06Scene.tsx`
- `src/remotion/pages/Page07Scene.tsx`
- `src/remotion/pages/Page08Scene.tsx`
- `src/remotion/pages/Page09Scene.tsx`

Each page file renders only the elements owned by that page's idea, even if those elements participate in page-to-page transitions.

Rule:

- previous-page carryover that still exists visually stays owned by the page that introduced it, unless ownership was already effectively transferred in the story
- late-page overlays such as external proof cards or PSO hash evidence stay in the page that introduces the concept

### 4. Keep `Composition.tsx` as a thin entry point

The target `Composition.tsx` should only:

- read `frame`
- compute the global scene model
- compose page renderers in order
- export `SceneSvg` and `MyComposition`

It should stop being the place where all constants, all helpers, and all render branches live.

## Directory Target

```text
SlideApp/src/remotion/
  Composition.tsx
  embed.ts
  sceneTimeline.ts
  geometry/
    geometry.ts
  model/
    computeSceneModel.ts
    scene-model-types.ts
  pages/
    Page01Scene.tsx
    Page02Scene.tsx
    Page03Scene.tsx
    Page04Scene.tsx
    Page05Scene.tsx
    Page06Scene.tsx
    Page07Scene.tsx
    Page08Scene.tsx
    Page09Scene.tsx
    page-layout-constants.ts
  primitives/
    diagramPrimitives.tsx
    diagramTypes.ts
```

## Typography Policy

The refactor should introduce a small shared token layer for diagram text sizes that are expected to be PPT-safe:

- node title
- stacked node label
- pill label
- helper note
- badge id

This is not a full design-system exercise. The goal is only to stop scattering key readability sizes across the scene.

## Why This Is Better Than Smaller Alternatives

### Rejected: Only split utilities

This would reduce file size, but it would not fix the main communication problem: page intent would still be buried inside one giant render function.

### Rejected: Only split pages but keep all model math in `Composition.tsx`

This would improve file navigation slightly, but the real complexity would still stay centralized and painful to edit.

### Rejected: Fully independent per-page models

This sounds attractive, but it would duplicate shared transition math and make cross-page continuity easier to break. A single scene model is more honest for this deck.

## Success Criteria

1. `Composition.tsx` becomes a thin orchestration file instead of a 4000-line scene monolith.
2. Every page has a dedicated renderer file.
3. Shared SVG primitives and geometry helpers are extracted.
4. The scene still renders and tests pass without visual ownership bugs.
5. Future edits to page 08, for example, should mostly touch `Page08Scene.tsx`, shared primitives, or page layout constants instead of a global render blob.
