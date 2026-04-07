# SlideApp InlineCode Two-Page Split Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Split the current InlineCode slide into two presentation-grade slides: one for layer ownership of shader variants, one for the runtime InlineCode lookup path.

**Architecture:** Keep the existing slide deck skeleton and Remotion transition system, but insert a new page boundary inside the current InlineCode sequence. The first slide emphasizes the three selector tables and their ownership layers; the second slide removes those left-side hints and reallocates the stage to a larger `FShaderMapResource_InlineCode` storage view. Update storyboard content, Remotion scene layout, and tests together so the new sequence remains spatially stable and verifiable.

**Tech Stack:** React 19, Remotion, Vite, Vitest, TypeScript

---

### Task 1: Update storyboard copy for the new two-slide teaching split

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/storyboard-data/pso-storyboard.ts`
- Test: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Write the failing test**

Add or update storyboard assertions so the deck expects:

- one slide focused on selector ownership
- one slide focused on InlineCode runtime lookup
- later PSO / SharedCode slides shifted accordingly

Use assertions like:

```ts
expect(page6.caption).toContain("区分");
expect(page7.caption).toContain("InlineCode");
```

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: FAIL because the old slide text and count still reflect the one-page InlineCode layout.

**Step 3: Write minimal implementation**

Update `pso-storyboard.ts` so:

- old page 06 becomes “区分因素在哪一层”
- new page 07 becomes “InlineCode 如何拿到 code”
- old PSO cache and SharedCode pages shift by one slot if needed
- `caption` / `notes` / `timingHint` / `intro` / `manuscript` match the approved split

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add src/storyboard-data/pso-storyboard.ts src/storyboard/pso-storyboard.test.ts
git commit -m "feat: split inlinecode storyboard into two slides"
```

### Task 2: Add failing Remotion tests for the two new page contracts

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add focused tests for:

- ownership slide:
  - selector tables visible
  - `Material / FMaterialResource / FMaterialShaderMap` visible
  - `FShaderMapResource_InlineCode` weak or minimal
  - `ShaderEntries[i]` and `Cooked ShaderCode` not yet expanded
- runtime lookup slide:
  - selector tables absent
  - shadow cards absent
  - `FShaderMapResource_InlineCode` large and primary
  - `ShaderEntries[i]`, `ShaderHashes[i]`, `Cooked ShaderCode` visible and roomy

Use assertions like:

```ts
expect(container.querySelector('[data-testid="page6-platform-table"]')).not.toBeNull();
expect(screen.queryByText("ShaderEntries[i]")).toBeNull();
```

and

```ts
expect(container.querySelector('[data-testid="page7-platform-table"]')).toBeNull();
expect(screen.getByText("ShaderEntries[i]")).toBeInTheDocument();
```

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "ownership slide|runtime lookup slide"`

Expected: FAIL because the current layout still merges both concepts into one page.

**Step 3: Write minimal implementation**

Do not implement yet. Only keep the failing expectations in place.

**Step 4: Run test to verify it still fails for the right reason**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "ownership slide|runtime lookup slide"`

Expected: FAIL with missing or mismatched page structure.

**Step 5: Commit**

```bash
git add src/remotion/Composition.test.tsx
git commit -m "test: define two-slide inlinecode expectations"
```

### Task 3: Split the page timing and scene identity in Remotion

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.tsx`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/embed.ts`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/sceneTimeline.test.ts`

**Step 1: Write the failing test**

Add a scene timeline test that expects a new page boundary between the current InlineCode explanation and the later PSO cache content.

Example assertion:

```ts
expect(resolveRemotionStepFrame("page_07")).toBeGreaterThan(resolveRemotionStepFrame("page_06"));
```

and ensure downstream frames remain ordered after insertion.

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/sceneTimeline.test.ts`

Expected: FAIL because the frame/timeline config does not yet include the extra split.

**Step 3: Write minimal implementation**

Update the scene timing source so:

- the ownership slide has its own settled page
- the runtime lookup slide has its own settled page
- downstream pages shift consistently

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/remotion/sceneTimeline.test.ts`

Expected: PASS

**Step 5: Commit**

```bash
git add src/remotion/Composition.tsx src/remotion/embed.ts src/remotion/sceneTimeline.test.ts
git commit -m "feat: add second inlinecode page timing"
```

### Task 4: Implement the ownership slide layout

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.tsx`
- Test: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

If not already covered, add exact geometry expectations for the ownership slide:

- three selector tables aligned on the left lane
- `Material -> FMaterialResource -> FMaterialShaderMap` readable in the center lane
- right side does not fully expand the storage internals

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "ownership slide"`

Expected: FAIL

**Step 3: Write minimal implementation**

Implement the new ownership slide in `Composition.tsx` by:

- keeping the selector tables and primary boxes large
- suppressing the detailed right-side storage expansion
- preserving continuity from the prior page

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "ownership slide"`

Expected: PASS

**Step 5: Commit**

```bash
git add src/remotion/Composition.tsx src/remotion/Composition.test.tsx
git commit -m "feat: add ownership-focused inlinecode slide"
```

### Task 5: Implement the runtime lookup slide layout

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.tsx`
- Test: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add exact expectations for the lookup slide:

- selector tables removed
- shadow cards removed
- `FShaderMapResource_InlineCode` enlarged
- `ShaderEntries[i]` and `ShaderHashes[i]` vertically heavier and easier to read
- `Cooked ShaderCode` horizontally aligned to the `ShaderEntries[i]` path

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "runtime lookup slide"`

Expected: FAIL

**Step 3: Write minimal implementation**

Implement the second slide so:

- left-side ownership hints are gone
- `FMaterialShaderMap`, `FShader`, and `ResourceIndex = i` remain as minimal anchors
- right-side storage becomes the dominant focus cluster

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "runtime lookup slide"`

Expected: PASS

**Step 5: Commit**

```bash
git add src/remotion/Composition.tsx src/remotion/Composition.test.tsx
git commit -m "feat: add runtime lookup inlinecode slide"
```

### Task 6: Re-flow downstream PSO cache and SharedCode pages

**Files:**
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.tsx`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/remotion/Composition.test.tsx`
- Modify: `/Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp/src/App.test.tsx`

**Step 1: Write the failing test**

Update tests that currently assume:

- old page numbers for PSO cache and SharedCode
- old keyboard navigation counts
- old settled slide content order

**Step 2: Run test to verify it fails**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "page 07|page 08|SharedCode|PSO Cache"`

Expected: FAIL because downstream assumptions still match the old sequence.

**Step 3: Write minimal implementation**

Shift downstream scene references and labels so the deck remains coherent after the new insertion.

**Step 4: Run test to verify it passes**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "page 07|page 08|SharedCode|PSO Cache"`

Expected: PASS

**Step 5: Commit**

```bash
git add src/remotion/Composition.tsx src/remotion/Composition.test.tsx src/App.test.tsx
git commit -m "feat: reflow downstream slides after inlinecode split"
```

### Task 7: Full verification and visual review

**Files:**
- Modify: none unless review finds issues

**Step 1: Run the full automated suite**

Run:

```bash
npm test
npm run build
```

Expected: all tests pass and production build succeeds.

**Step 2: Run visual spot checks**

Run the local app and inspect:

- previous page -> ownership slide
- ownership slide -> runtime lookup slide
- runtime lookup slide -> PSO cache

Save screenshots if needed for review.

**Step 3: Fix any blocker found**

If visual or layout regressions appear, make the smallest change that preserves the approved page contract.

**Step 4: Re-run verification**

Run:

```bash
npm test
npm run build
```

Expected: PASS

**Step 5: Commit**

```bash
git add .
git commit -m "chore: verify inlinecode two-slide split"
```
