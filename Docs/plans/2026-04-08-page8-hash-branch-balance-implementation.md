# Page 08 Hash Branch Balance Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Re-balance slide page 08 so the `PSO Cache` and red proof branches support the existing InlineCode lookup story instead of overwhelming it.

**Architecture:** Preserve page 07's upper skeleton and only adjust page 08-specific layout primitives: the lower `PSO Cache` box, the red dashed hash-reference branch, and the outside `Material` proof cluster. Lock the intended relationships in tests first, then make the smallest coordinate and routing changes needed to satisfy them.

**Tech Stack:** React, Remotion, TypeScript, Vitest, Testing Library

---

### Task 1: Add failing layout tests for the page 08 balance contract

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing tests**

Add assertions that:
- the page 08 `PSO Cache` box is narrower than the page 07 carry-over cache box and remains centered below the `uasset`
- the two red hash-reference arrows stay visually inside the `ShaderHashes[idx]` band rather than spreading across the whole payload width
- the outside `Material` proof box stays local to `Cooked ShaderCode` and does not overlap the broader hash-branch span

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test -- --run src/remotion/Composition.test.tsx -t "page 08"`

Expected: FAIL because the current page 08 layout still gives `PSO Cache` too much span and the proof layers are not yet re-balanced.

### Task 2: Adjust page 08 constants and routing

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Re-size and re-position the page 08 PSO layer**

- narrow `PAGE8_PSO_BOX`
- keep it below the `uasset`
- center it under the meaningful hash story, not across the entire canvas

**Step 2: Tighten the hash proof branch**

- keep the dashed red stem orthogonal
- reduce the horizontal spread between the two proof arrows
- keep the branch visually tied to `ShaderHashes[idx]`

**Step 3: Keep the outside Material proof local**

- leave the page 07 upper skeleton untouched
- only move the outside proof cluster if needed so it stays near `Cooked ShaderCode`
- ensure the proof arrow still reads as a local red evidence line

**Step 4: Run focused test**

Run: `cd SlideApp && npm test -- --run src/remotion/Composition.test.tsx -t "page 08"`

Expected: PASS

### Task 3: Verify page-sequence stability

**Files:**
- Modify: none unless verification finds a regression

**Step 1: Run the full composition tests**

Run: `cd SlideApp && npm test -- --run src/remotion/Composition.test.tsx`

Expected: PASS

**Step 2: Run the full SlideApp test suite**

Run: `cd SlideApp && npm test`

Expected: PASS

**Step 3: Run the production build**

Run: `cd SlideApp && npm run build`

Expected: PASS
