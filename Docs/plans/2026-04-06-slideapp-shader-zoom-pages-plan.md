# Slide 06-08 Shader Zoom Pages Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Extend the slide app from page 05 to pages 06-08 with a question-mark zoom transition that explains InlineCode internals, why PSO cache stores hashes, and why SharedCode becomes necessary.

**Architecture:** Keep the existing Remotion deck and left-note structure intact, add three new storyboard steps, extend the frame timeline, and implement the new visuals inside the existing SVG composition so pages 05-08 feel like one continuous zoomed-in narrative rather than unrelated slides. Reuse the current node language, badge language, and orange/green semantic colors.

**Tech Stack:** React, Remotion, Vitest, Testing Library, TypeScript

---

### Task 1: Extend Step Metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Write the failing test**

Update storyboard tests to expect `page_06`, `page_07`, and `page_08`, plus the new labels and manuscript keywords:
- `InlineCode 内部结构`
- `PSO cache 为什么只存 Hash`
- `SharedCode 为什么成为必需`

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test src/storyboard/pso-storyboard.test.ts`
Expected: FAIL because only pages 01-05 exist.

**Step 3: Write minimal implementation**

- Extend `StoryStepId` with `page_06`, `page_07`, `page_08`
- Add three new steps to `masterStoryboard`
- Keep all text in Chinese except UE/OpenGL/Vulkan/API terms
- Use `ShaderCode教程.md` chapter 1-2 concepts to describe:
  - `Material -> FMaterialResource -> FShaderMap -> InlineCode`
  - `PSO cache` storing `Hash / key / metadata`
  - `SharedCode Library` solving duplication across many materials

**Step 4: Run test to verify it passes**

Run: `cd SlideApp && npm test src/storyboard/pso-storyboard.test.ts`
Expected: PASS

### Task 2: Extend Timeline And Navigation

**Files:**
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/App.test.tsx`

**Step 1: Write the failing test**

Update app tests to expect:
- 8 progress steps instead of 5
- direct selection support for pages 06-08
- keyboard navigation can reach the new pages

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test src/App.test.tsx`
Expected: FAIL because step count and step ids still stop at page 05.

**Step 3: Write minimal implementation**

- Extend `STEP_FRAME_MAP` to pages 06-08
- Increase Remotion duration accordingly
- Extend `REMOTION_STEP_SEQUENCE`
- Keep the existing frame spacing rhythm unless a different spacing is necessary for a smooth zoom

**Step 4: Run test to verify it passes**

Run: `cd SlideApp && npm test src/App.test.tsx`
Expected: PASS

### Task 3: Add Failing Composition Tests For Pages 05-08

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add composition tests for:
- page 05 has a visible orange `?` badge on the `Material -> Cooked ShaderCode` relation
- page 06 zooms into that question area and shows `Material`, `FMaterialResource`, `FShaderMap`, and `InlineCode`
- page 07 keeps the zoomed area and shows `PSO Cache`, `Hash`, and keeps actual code storage separate
- page 08 shows `SharedCode Library` with multiple upstream sources converging into it

**Step 2: Run test to verify it fails**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: FAIL because those pages and nodes do not exist yet.

**Step 3: Keep tests minimal**

Prefer checking:
- node presence
- a few structural arrows / frames / badges
- continuity of the question-to-zoom handoff

**Step 4: Run test to verify the failures are correct**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: FAIL for missing new visuals, not typos.

### Task 4: Implement Page 05 Question Hook

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Add the page 05 hook**

- Add a small orange `?` marker on the `Material -> Cooked ShaderCode` relation
- Keep page 05 spine, assets, and color semantics intact
- Make the `?` visually readable without overwhelming the page

**Step 2: Run targeted tests**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: the page 05 question test passes, later page tests still fail.

### Task 5: Implement The Page 05 -> 06 Zoom Transition

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Build the zoom language**

- Treat the page 05 `?` region as the zoom anchor
- Scale and pan that local region into a larger “analysis canvas”
- Preserve visual continuity so page 06 feels like a magnified subsection of page 05

**Step 2: Keep the scene readable**

- Fade or de-emphasize unrelated page 05 elements while zooming
- Preserve the current palette and rounded-box visual language

**Step 3: Run targeted tests**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: page 06 presence tests get closer or pass.

### Task 6: Implement Page 06 InlineCode Internals

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Add the core chain**

- `Material`
- `FMaterialResource`
- `FShaderMap`
- `InlineCode blob`

**Step 2: Clarify semantics**

- Show that `Material` does not directly own ready-to-bind PSO
- Show that cooked shader code sits inside per-material / per-resource packaging

**Step 3: Run targeted tests**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: page 06 tests pass.

### Task 7: Implement Page 07 PSO Cache Hash Story

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Add page 07 structure**

- keep the zoom canvas
- show `PSO Cache`
- show `Hash / key / metadata`
- visually separate cached identifiers from actual code storage

**Step 2: Explain the constraint**

- make it visually obvious that the cache is pointing at or indexing code/state, not embedding the full shader blob repeatedly

**Step 3: Run targeted tests**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: page 07 tests pass.

### Task 8: Implement Page 08 SharedCode Necessity

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Evolve page 07 into page 08**

- transform `InlineCode blob` into `SharedCode Library`
- show multiple sources converging
- show that PSO-related structures can point to shared code instead of copying binaries everywhere

**Step 2: Preserve continuity**

- avoid a hard redraw
- morph labels and routing where semantics continue

**Step 3: Run targeted tests**

Run: `cd SlideApp && npm test src/remotion/Composition.test.tsx`
Expected: page 08 tests pass.

### Task 9: Run Full Verification

**Files:**
- No code changes expected

**Step 1: Run all tests**

Run: `cd SlideApp && npm test`
Expected: PASS

**Step 2: Run production build**

Run: `cd SlideApp && npm run build`
Expected: PASS

**Step 3: Sanity-check the final deck**

- confirm pages 05-08 preserve the established visual language
- confirm the zoom anchor is still the page 05 question marker
- confirm the new pages teach one new idea each
