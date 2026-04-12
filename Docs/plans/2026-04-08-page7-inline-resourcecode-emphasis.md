# Page 07 Inline ResourceCode Emphasis Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Re-layout page 07 so the left ownership spine stays lightweight while `FShaderMapResourceCode` becomes the real visual focus inside `FShaderMapResource_InlineCode`.

**Architecture:** Keep the approved page 06 to page 07 structural continuity, but strip page 07 down to a smaller left spine, a centered `FShaderMapResource_InlineCode` receiver plane, and a larger inner `FShaderMapResourceCode` payload box. Use `idx` as the runtime callout label, keep it off the vertical line, and ensure `Cooked ShaderCode` sits fully outside the receiver plane.

**Tech Stack:** React, Remotion SVG composition, Vitest regression tests.

---

### Task 1: Lock page 07 geometry in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**
- Add assertions that the page 07 index callout label is `idx`.
- Assert that the `idx` callout sits beside the vertical arrow rather than centered on it.
- Assert that `FShaderMapResourceCode` is larger inside `FShaderMapResource_InlineCode`.
- Assert that `Cooked ShaderCode` starts to the right of the outer `InlineCode` box.

**Step 2: Run test to verify it fails**
Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL on old `i` label / old sizing / overlap assumptions.

### Task 2: Update page 07 layout

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write minimal implementation**
- Shrink and left-shift the page 07 ownership spine.
- Change the line callout from `i` to `idx`, positioned off the arrow centerline.
- Keep the outer `FShaderMapResource_InlineCode` stable enough for continuity.
- Enlarge `FShaderMapResourceCode` and its internal pills.
- Push `Cooked ShaderCode` fully outside the outer receiver plane.

**Step 2: Run focused tests**
Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 3: Full verification

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Run complete checks**
Run: `npm test`
Expected: PASS.

**Step 2: Run production build**
Run: `npm run build`
Expected: PASS.
