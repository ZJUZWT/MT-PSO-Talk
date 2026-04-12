# Page 06-07 Inline Index Layout Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Re-layout page 06 so ownership flows vertically on the left, index lookup lives on the right, and page 07 reuses `ShaderHashes[i]` as the red problem explanation target.

**Architecture:** Page 06 becomes a two-lane diagram: a left vertical ownership lane with selector tables on the outside-left, and a right lookup/storage lane where `FShader` accesses `FShaderMapResource_InlineCode` via index `i`. Page 07 keeps the cache story but retargets the VS/FS explanation arrows to `ShaderHashes[i]` so the causal bridge remains explicit.

**Tech Stack:** React, SVG layout in Remotion composition, Vitest regression tests.

---

### Task 1: Lock the new page 06/07 layout in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**
- Add assertions for vertical left column ordering of `Material`, `FMaterialResource`, `FMaterialShaderMap`.
- Add assertions that the selector tables sit to the left of their matching nodes.
- Add assertions that `FShader` contains `ResourceIndex`, the lookup arrow targets `InlineCode`, the arrow carries `i`, and the internal labels use `[i]`.
- Add assertions that page 07 red arrows target `ShaderHashes[i]`.

**Step 2: Run test to verify it fails**
Run: `npm test -- -t "renders page 06"`
Expected: FAIL on old horizontal layout assumptions.

### Task 2: Re-layout page 06 composition

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write minimal implementation**
- Reposition page 06 boxes into a left vertical lane and right lookup lane.
- Move selector tables to the left side of each ownership node.
- Collapse `FShader` label/content so `ResourceIndex` is inside the node.
- Route `FMaterialShaderMap -> FShaderMapResource_InlineCode` horizontally.
- Route `FShader -> InlineCode` with an `i` callout on the line.
- Rename internals to `ShaderEntries[i]` / `ShaderHashes[i]`.
- Add leftward `ShaderEntries[i] -> CookedShaderCode` output.

**Step 2: Run focused tests**
Run: `npm test -- -t "page 06"`
Expected: PASS.

### Task 3: Reconnect page 07 problem arrows

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Minimal implementation**
- Retarget the red VS/FS explanatory arrows to the page 06 `ShaderHashes[i]` anchor.
- Keep page 07 cache layout intact unless spacing forces a minor move.

**Step 2: Full verification**
Run: `npm test && npm run build`
Expected: all tests pass and production build succeeds.
