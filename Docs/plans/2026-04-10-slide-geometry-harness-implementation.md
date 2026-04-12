# Slide Geometry Harness Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a first-pass `slide-geometry-harness` to `SlideApp` so a short page contract can become a browser-runnable sketch URL, a latest screenshot, and a fact-bound dual-critic review loop.

**Architecture:** Reuse the existing `SlideApp` review-mode shell, URL query plumbing, and capture utilities instead of building a second preview app. Physically isolate the harness under `SlideApp/src/harness/slide-geometry` and any harness-only scripts under `scripts/slide-geometry-harness`, expose sketch mode through query params on the existing port, and define the critic prompts and scoring policy in repo docs so the builder loop can hand the latest screenshot to two isolated review agents.

**Tech Stack:** React, TypeScript, Vitest, existing `html-to-image` capture utility, Markdown skill docs.

**Execution Constraint:** Implement directly in the current `main` workspace. Do not create a feature branch or a separate worktree for this task.

---

### Task 1: Lock sketch-mode routing and latest-only review output in tests

**Files:**
- Modify: `SlideApp/src/App.test.tsx`

**Step 1: Write the failing tests**

Add tests that:

- boot `App` from `/?mode=sketch&sketch=page09-r1&review=1`
- verify the app renders the sketch scene instead of the normal Remotion step scene
- verify the rendered review surface exposes the exact sketch URL for copy/open workflows
- verify the review capture path still targets the stage-only element in sketch mode

**Step 2: Run targeted tests to verify failure**

Run: `cd SlideApp && npm test -- src/App.test.tsx -t "sketch mode|latest review|stage-only"`

Expected: FAIL because sketch-mode routing and sketch-stage rendering do not exist yet.

### Task 2: Add a data contract for geometry sketches

**Files:**
- Create: `SlideApp/src/harness/slide-geometry/render/geometry-sketch-types.ts`
- Create: `SlideApp/src/harness/slide-geometry/contracts/page09-r1.ts`
- Create: `SlideApp/src/harness/slide-geometry/registry/sketchRegistry.ts`

**Step 1: Define sketch types**

Create explicit types for:

- sketch id
- page contract fields
- node boxes
- orthogonal edges
- optional lane or band hints

**Step 2: Create one real sample contract**

Add `page09-r1` as the initial sketch specimen so the harness has one browser-runnable page immediately.

**Step 3: Register sketches**

Expose a registry keyed by `sketch` query param so the app can resolve sketch ids deterministically.

### Task 3: Build a generic geometry sketch renderer

**Files:**
- Create: `SlideApp/src/harness/slide-geometry/render/GeometrySketchScene.tsx`
- Modify: `SlideApp/src/app.css`

**Step 1: Render minimal sketch primitives**

Support:

- stage-sized contract-only cards
- orthogonal lines
- simple pills or labels
- one clearly emphasized receiver plane

**Step 2: Keep the sketch visibly draft-like**

Do not mimic final slide styling. Use restrained draft visuals so geometry judgment stays primary.

**Step 3: Add sketch-only CSS hooks**

Keep sketch styles isolated so the production workbench shell is not polluted.

### Task 4: Integrate sketch mode into the existing `SlideApp` shell

**Files:**
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/components/StageFrame.tsx`
- Modify: `SlideApp/src/components/ReviewHud.tsx`

**Step 1: Parse sketch query params**

Add support for:

- `mode=sketch`
- `sketch=<sketch-id>`

Keep existing review-mode behavior intact.

**Step 2: Render either the normal stage or the sketch stage**

The shell should keep one capture surface and one review HUD, but switch the stage content depending on mode.

**Step 3: Surface the exact latest sketch URL**

Show the current sketch URL in review mode so the main agent can report it directly.

### Task 5: Add fact and metric extraction helpers for non-hallucinated scoring

**Files:**
- Create: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- Create: `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts`
- Create: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts`

**Step 1: Write failing metric tests**

Cover simple deterministic cases for:

- overlap detection
- crossing detection
- primary-line bend counts
- minimum gap calculations

**Step 2: Run the targeted tests**

Run: `cd SlideApp && npm test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: FAIL because the metric helpers do not exist yet.

**Step 3: Implement the minimal helpers**

Keep them data-driven over the sketch spec. The goal is bounded evidence, not a full layout solver.

**Step 4: Add score mapping**

Translate metrics into capped score bands so critic output has a mechanical floor.

### Task 6: Document the dual-critic harness contract

**Files:**
- Create: `Docs/SkillFramework/higher-level/slide-geometry-harness/SKILL.md`
- Create: `Docs/SkillFramework/higher-level/slide-geometry-harness/references/loop-contract.md`
- Create: `Docs/SkillFramework/higher-level/slide-geometry-harness/references/critic-prompts.md`
- Modify: `Docs/SkillFramework/lower-level/analyze-image-aesthetics/SKILL.md`

**Step 1: Define the harness workflow**

Document:

- builder does not self-grade
- capture is mechanical
- art and geometry critics run in parallel
- critics receive latest-only artifacts
- the loop caps at three rounds

**Step 2: Write the blind critic prompts**

Separate:

- art critic prompt
- geometry critic prompt

Both prompts must forbid reading builder rationale and must require facts before scores.

**Step 3: Narrow the older image-aesthetic skill**

Update `analyze-image-aesthetics` so it clearly stays generic, while the slide harness owns this slide-specific dual-critic loop.

### Task 7: Verify end-to-end

**Files:**
- Verify: `SlideApp/src/App.test.tsx`
- Verify: `SlideApp/src/harness/slide-geometry/review/geometryMetrics.test.ts`

**Step 1: Run targeted sketch tests**

Run: `cd SlideApp && npm test -- src/App.test.tsx -t "sketch mode|latest review|stage-only"`

Expected: PASS.

**Step 2: Run targeted metric tests**

Run: `cd SlideApp && npm test -- src/harness/slide-geometry/review/geometryMetrics.test.ts`

Expected: PASS.

**Step 3: Run the full app test suite**

Run: `cd SlideApp && npm test`

Expected: PASS.

**Step 4: Run the production build**

Run: `cd SlideApp && npm run build`

Expected: PASS.
