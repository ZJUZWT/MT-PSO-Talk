# PSO Video Diagram Refinement Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Refine the PSO web demo so the right-side stage uses space better, arrows stay proportional, and the inline/shared-code beats use cleaner bottom-up bus layouts.

**Architecture:** Update the shared visual contract in the Remotion composition and the Motion Canvas scene together so both renderers expose the same improved geometry. Add one focused regression test for the revised bridge telemetry and then verify the behavior with the existing workbench tests plus browser smoke screenshots.

**Tech Stack:** React, Remotion, Motion Canvas, Vitest, Vite, Playwright smoke scripts.

---

### Task 1: Lock one new visual contract in a failing test

**Files:**
- Modify: `Docs/prototypes/workbench/src/adapters/motionCanvasEmbedBridge.test.ts`
- Test: `Docs/prototypes/workbench/src/adapters/motionCanvasEmbedBridge.test.ts`

**Step 1: Write the failing test**

Add an assertion that the Motion Canvas embed bridge emits frame telemetry back
to the host:

```ts
it("posts current-frame telemetry back to the host", () => {
  expect(motionCanvasEmbedSource).toContain('type: "pso-workbench:frame"');
});
```

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/adapters/motionCanvasEmbedBridge.test.ts`

Expected: FAIL until the embed source reflects the final bridge contract.

### Task 2: Enlarge and rebalance the workbench stage

**Files:**
- Modify: `Docs/prototypes/workbench/src/app.css`

**Step 1: Write minimal implementation**

- Increase the stage card footprint inside the right panel
- Reduce overly constrained max widths in the runtime and embed containers
- Keep mobile behavior intact

**Step 2: Run targeted verification**

Run: `npm --prefix Docs/prototypes/workbench run build`

Expected: PASS with updated CSS bundling successfully.

### Task 3: Refine the Remotion diagram geometry

**Files:**
- Modify: `Docs/prototypes/remotion-demo/src/Composition.tsx`

**Step 1: Write minimal implementation**

- Reduce marker size and retune line endpoints
- Expand the usable panel footprint
- Re-route `Step 3` through a lower `ShaderCode` bus
- Remove inline leftovers from `Step 4`
- Add a single orthogonal `ShaderCodeLib -> ShaderCode` accent line from below

**Step 2: Run verification**

Run: `npm --prefix Docs/prototypes/workbench run build`

Expected: PASS with Remotion still compiling through the workbench build.

### Task 4: Mirror the same geometry in Motion Canvas

**Files:**
- Modify: `Docs/prototypes/motion-canvas-demo/src/scenes/example.tsx`
- Modify: `Docs/prototypes/motion-canvas-demo/src/embed.ts`

**Step 1: Write minimal implementation**

- Match the Remotion stage occupancy changes
- Shrink arrowheads and tune line spacing
- Rebuild the inline/shared-code paths with the same bottom-up bus model
- Keep bridge frame telemetry intact

**Step 2: Run verification**

Run: `npm --prefix Docs/prototypes run build`

Expected: PASS with both workbench and Motion Canvas production builds
succeeding.

### Task 5: Run the full verification pass

**Files:**
- Output: `Docs/prototypes/.smoke-artifacts/animated-flip-v2/`

**Step 1: Run tests**

Run: `npm --prefix Docs/prototypes run test`

Expected: PASS.

**Step 2: Run browser smoke**

Run a Playwright smoke script against `http://127.0.0.1:4173` and verify:

- Remotion mid-frame differs from its final frame
- Motion Canvas mid-frame differs from its final frame
- The `Step 3` bus routes upward into `ShaderCode`
- The `Step 4` shared path has no dashed inline leftovers

**Step 3: Capture artifacts**

Save refreshed screenshots under:

- `Docs/prototypes/.smoke-artifacts/animated-flip-v2/remotion-mid.png`
- `Docs/prototypes/.smoke-artifacts/animated-flip-v2/remotion-after.png`
- `Docs/prototypes/.smoke-artifacts/animated-flip-v2/motion-mid.png`
- `Docs/prototypes/.smoke-artifacts/animated-flip-v2/motion-after.png`
