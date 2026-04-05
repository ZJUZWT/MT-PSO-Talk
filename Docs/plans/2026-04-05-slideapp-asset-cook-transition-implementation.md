# SlideApp Asset Cook Transition Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Extend the reduced SlideApp storyboard to five steps and reframe the later pages around raw shader code, SPIR-V, and UE-style asset cooking.

**Architecture:** Update the storyboard data first so the semantic chain is explicit and testable. Then evolve the Remotion composition to support a fifth frame, compact two-line node labels, and a distinct asset node style used by the new `Mesh` and `Material` nodes on the final page.

**Tech Stack:** React, TypeScript, Remotion, Vitest.

---

### Task 1: Lock the new five-step storyline with tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/App.test.tsx`

**Step 1: Write failing storyboard expectations**

Add assertions that:

- step order is now `page_01` through `page_05`
- page 03 references `Raw ShaderCode` and `Binary ShaderCode`
- page 04 references `Raw ShaderCode` and `SPIR-V ShaderCode`
- page 05 introduces `Mesh`, `Material`, and `Cooked ShaderCode`

**Step 2: Run the focused tests to verify they fail**

Run: `npm test -- src/storyboard/pso-storyboard.test.ts src/App.test.tsx`

Expected: FAIL because only four steps exist and the new page semantics are missing.

### Task 2: Update storyboard types and step metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/embed.ts`

**Step 1: Add the fifth step id**

Extend `StoryStepId` with `page_05`.

**Step 2: Rewrite page 03/04 copy**

Update labels, captions, notes, and manuscripts to match:

- page 03: `Raw ShaderCode -> Binary ShaderCode`
- page 04: `Raw ShaderCode -> SPIR-V ShaderCode`

**Step 3: Add page 05**

Create metadata for the new asset/cook page with:

- `Mesh -> VertexData`
- `Material -(cook)-> Cooked ShaderCode`
- return to `GPU -> Pixels`

**Step 4: Update Remotion step frame map**

Assign a fifth frame anchor for `page_05`.

**Step 5: Re-run the focused tests**

Run: `npm test -- src/storyboard/pso-storyboard.test.ts src/App.test.tsx`

Expected: storyline tests still fail only because the composition and UI have not caught up yet, or pass if the assertions only target data.

### Task 3: Add compact multiline labels and asset node styling

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write a rendering-oriented failing test if needed**

If there is a stable test seam, add or extend a test to confirm the fifth page can be selected and rendered without falling back.

**Step 2: Extend the box label renderer**

Update `StageBox` so labels can render one or two lines without changing the box geometry unpredictably.

**Step 3: Add asset node styling**

Create a distinct visual treatment for authored assets such as:

- different fill/stroke pairing
- subtle badge/glyph or inset treatment

Use it only for `Mesh` and `Material`.

### Task 4: Replace the final PSO packaging page with the asset/cook bridge page

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Keep page 03 in OpenGL mode**

Rename the shader lane and preserve the current OpenGL setup behavior.

**Step 2: Keep page 04 in Vulkan mode**

Rename the Vulkan shader lane and preserve the raw-to-SPIR-V reading.

**Step 3: Introduce page 05**

Remove `Description` and `PSO` from the final page state.

Render a new end state that shows:

- `Mesh -> VertexData`
- `Material -(cook)-> Cooked ShaderCode`
- `VertexData -> GPU`
- `Cooked ShaderCode -> GPU`
- `GPU -> Pixels`

### Task 5: Verify end-to-end

**Files:**
- Verify: `SlideApp/`

**Step 1: Run full tests**

Run: `npm test`

Expected: PASS

**Step 2: Run production build**

Run: `npm run build`

Expected: PASS
