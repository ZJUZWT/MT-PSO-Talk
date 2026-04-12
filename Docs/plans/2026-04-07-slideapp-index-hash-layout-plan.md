# SlideApp Index-Hash Layout Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Reframe SlideApp pages 06-08 so the runtime lookup path is driven by `ResourceIndex` and `ShaderMapIndex`, while `ShaderHash` appears as the PSO-facing side branch that later enables hash-to-code lookup.

**Architecture:** Keep the current page 05-08 zoom narrative and stage coordinates, but replace the page 06 right-side blob with a more explicit `FShader -> ResourceIndex -> FShaderMapResource_InlineCode` chain plus an inline storage detail branch. Then evolve that same branch on page 08 into `FShaderMapResource_SharedCode`, introduce `ShaderMapIndex`, and route `ResourceIndex + ShaderMapIndex` into a `ShaderCodeLibrary / ShaderArchive` box while preserving the existing PSO cache branch.

**Tech Stack:** React, Remotion, TypeScript, Vitest, Testing Library

---

### Task 1: Lock The New Page 06-08 Structure In Tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add assertions that:
- page 06 shows `FShader`, `ResourceIndex`, `FShaderMapResource_InlineCode`, `FShaderMapResourceCode`, `ShaderEntries[Index]`, and `ShaderHashes[Index]`
- page 06 still shows `ShaderCode`, but now as the terminus of the inline storage branch
- page 08 shows `FShaderMapResource_SharedCode`, `ShaderMapIndex`, and a library lookup path
- page 08 still keeps `PSO Cache`, but the hash side branch is visibly distinct from the main code lookup chain

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL because the current layout still uses the older simplified inline/shared boxes.

**Step 3: Keep the tests structural**

Prefer checking:
- node presence
- a few key arrows or boxes
- continuity between page 06, page 07, and page 08

**Step 4: Run test to verify the failures are correct**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL for missing new runtime/index/hash structure, not for typos.

### Task 2: Update Storyboard Copy To Match The New Runtime Story

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Adjust page 06 manuscript**

Explain that:
- `FShader` carries a local `ResourceIndex`
- the split happens at `FShaderMapResource`
- InlineCode reaches `FShaderMapResourceCode -> ShaderEntries[Index] -> ShaderCode`
- `ShaderHashes[Index]` exists, but is not the main runtime lookup path on this page

**Step 2: Adjust page 08 manuscript**

Explain that:
- SharedCode turns the split node into `FShaderMapResource_SharedCode`
- `ShaderMapIndex + ResourceIndex` resolve into the global library
- `ShaderHash` remains crucial for PSO cache lookup and precompile

**Step 3: Verify copy still fits the deck**

Run: `cd SlideApp && npm test -- src/App.test.tsx src/storyboard/pso-storyboard.test.ts`
Expected: PASS

### Task 3: Reshape The Page 06 Runtime Chain

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Preserve the upper inherited spine**

Keep:
- `Material`
- `FMaterialResource`
- `FShaderMap`
- `FShader`

Add:
- a clear `ResourceIndex` node on the main axis after `FShader`
- a downward branch from `ResourceIndex` to `FShaderMapResource_InlineCode`

**Step 2: Build the inline storage branch**

From `FShaderMapResource_InlineCode`, route into:
- `FShaderMapResourceCode`
- `ShaderEntries[Index]`
- `ShaderCode`

Also show:
- `ShaderHashes[Index]`

but keep it visually secondary so it reads as metadata / later-use context, not the main runtime path.

**Step 3: Re-check spacing and routing**

Keep:
- one readable main axis
- orthogonal routing
- stable page 06 stage center

**Step 4: Run focused test**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`
Expected: page 06 structure tests pass while page 08 still fails.

### Task 4: Evolve Page 08 Into SharedCode Lookup

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Morph the split node**

Turn:
- `FShaderMapResource_InlineCode`

into:
- `FShaderMapResource_SharedCode`

and add:
- `ShaderMapIndex`

**Step 2: Build the global lookup branch**

Route:
- `ResourceIndex + ShaderMapIndex`
- into `ShaderCodeLibrary / ShaderArchive`
- then to the shared `ShaderCode`

Inside or near the library show:
- `ShaderEntries[GlobalIndex]`
- `ShaderHashes[GlobalIndex]`

**Step 3: Keep PSO as a side branch**

Preserve the existing `PSO Cache` presentation, but make the page 08 relation read as:
- main path: index-based runtime/library lookup
- side path: hash-based PSO precompile lookup

**Step 4: Run focused test**

Run: `cd SlideApp && npm test -- src/remotion/Composition.test.tsx`
Expected: page 08 tests pass.

### Task 5: Full Verification

**Files:**
- No code changes expected

**Step 1: Run the full test suite**

Run: `cd SlideApp && npm test`
Expected: PASS

**Step 2: Run the production build**

Run: `cd SlideApp && npm run build`
Expected: PASS

**Step 3: Sanity-check the sequence**

Confirm:
- page 06 teaches runtime ownership and inline storage
- page 07 naturally introduces hash-bearing PSO metadata
- page 08 reuses that hash concept without letting it replace the main runtime lookup chain
