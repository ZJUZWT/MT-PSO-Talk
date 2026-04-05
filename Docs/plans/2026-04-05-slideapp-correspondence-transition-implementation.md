# SlideApp Correspondence-First Transition Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rework the `page_04 -> page_05` motion in `SlideApp` so UE asset cook reads as a correspondence-based evolution of the Vulkan page instead of a mostly opacity-driven page swap.

**Architecture:** First lock the transition contract in docs and the animation skill so the code change has explicit acceptance rules. Then add focused rendering tests around page 05 layout, asset styling, and mid-transition continuity. Finally update the Remotion composition so stable runtime skeleton nodes persist, shader nodes hand off into the cook view, and new asset nodes attach onto the inherited structure.

**Tech Stack:** React, TypeScript, Remotion, Vitest.

---

### Task 1: Lock the new transition contract in docs

**Files:**
- Create: `Docs/plans/2026-04-05-slideapp-correspondence-transition-design.md`
- Create: `Docs/plans/2026-04-05-slideapp-correspondence-transition-implementation.md`
- Modify: `Docs/skills/technical-talk-animation/SKILL.md`

**Step 1: Write the design doc**

Document:

- node / edge correspondence first
- stable skeleton vs. re-authored layers
- the `page_04 -> page_05` mapping
- banned motion patterns

**Step 2: Update the skill**

Add default rules for:

- stable skeleton preservation
- arrow width and routing consistency
- optical gap preservation
- morph / handoff preferred over fade when semantics continue

**Step 3: Review the docs against the benchmark**

Check the docs against `Docs/线条排版评估Benchmark.md` and make sure the rules align.

### Task 2: Add failing rendering tests for the asset-cook handoff

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Add a page 05 vertex identity test**

Assert that the final `page_05` frame still contains a visible `vertex-icon`, not only the `VertexData` label.

**Step 2: Add an asset-box purity test**

Assert that `Mesh` and `Material` use the pale green asset fill and do not render the old internal decorative rectangles.

**Step 3: Add a mid-transition correspondence test**

At a frame between `page_04` and `page_05`, assert that:

- `PSO` is already receding
- `Mesh` and `Material` have started appearing
- the shader target node is still present during the handoff
- the vertex icon remains visible during the transition

**Step 4: Run the focused test**

Run: `npm test -- src/remotion/Composition.test.tsx`

Expected: FAIL on the new assertions before production changes.

### Task 3: Refine the composition with correspondence-first motion

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Remove decorative asset chrome**

Delete the extra inner asset stripes from `StageBox` asset mode.

**Step 2: Restore vertex identity on page 05**

Keep the `VertexData` icon visible through the asset-cook page, while letting the label come in as a supporting annotation.

**Step 3: Keep Mesh on the left runtime attachment**

Preserve the horizontal `Mesh -> VertexData` attachment without changing the main runtime axis.

**Step 4: Rework page 04 -> 05 timing**

Replace the mostly opacity-based swap with a mapped handoff:

- keep the shader product node alive while it is renamed/reframed into `Cooked ShaderCode`
- delay complete removal of `PSO` until the new asset band is established
- keep arrow language and widths aligned with the inherited skeleton

**Step 5: Re-run the focused composition test**

Run: `npm test -- src/remotion/Composition.test.tsx`

Expected: PASS

### Task 4: Run full verification

**Files:**
- Verify: `SlideApp/`

**Step 1: Run the full test suite**

Run: `npm test`

Expected: PASS

**Step 2: Run the production build**

Run: `npm run build`

Expected: PASS
