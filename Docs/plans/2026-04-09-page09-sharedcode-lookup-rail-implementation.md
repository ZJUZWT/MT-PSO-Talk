# Page09 SharedCode Lookup Rail Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rebuild slide page 09 so it evolves naturally from page 08: the PSO cache stays on the inherited bottom band, while SharedCode becomes the new solution page with a cleaner lookup rail inside the global library.

**Architecture:** Lock the new page 09 contract in tests first, then replace the old “materials + bus + separate lookup arrows” rendering with a simpler layout. Keep page 08 geometry stable, reuse the existing scene timing where possible, and add only the new scene values needed for page 09’s library-focused composition.

**Tech Stack:** React, Remotion scene model, Vitest, Testing Library.

---

### Task 1: Lock page 09 behavior in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add assertions that page 09:
- keeps `PSO Cache` on the inherited bottom band
- renders `SharedCode Library`
- renders `ShaderMapEntries[ShaderMapIndex]`
- renders the lookup formula text
- does not render the old multi-material bus / Material A/B/C labels inside the stage content contract for page 09

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`

**Step 3: Write minimal implementation**

Update page 09 scene and any scene-model fields needed by the new layout.

**Step 4: Run test to verify it passes**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`

### Task 2: Rebuild page 09 rendering

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page09Scene.tsx`
- Modify: `SlideApp/src/remotion/model/computeSceneModel.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts` if new boxes are needed

**Step 1: Keep the inherited bottom band**

Reuse the existing page 08 PSO cache placement instead of moving the cache to a new band.

**Step 2: Make the library the receiver plane**

Render a larger `SharedCode Library` on the right and keep the left uasset chain visually quieter.

**Step 3: Replace node-heavy math with a lookup rail**

Represent the indexing logic as:
- `ShaderMapEntries[ShaderMapIndex]`
- formula text for `GlobalIndex = BaseOffset + ResourceIndex`
- `ShaderHashes[GlobalIndex]`
- `ShaderCode[GlobalIndex]`

**Step 4: Keep proof lines sparse**

Use one main lookup path plus the red dashed PSO proof lines, avoiding the old bus-like clutter.

### Task 3: Verify everything

**Files:**
- Verify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Run targeted test**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`

**Step 2: Run full test suite**

Run: `cd SlideApp && npm test`

**Step 3: Run build**

Run: `cd SlideApp && npm run build`
