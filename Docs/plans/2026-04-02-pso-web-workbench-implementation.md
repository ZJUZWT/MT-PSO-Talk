# PSO Web Workbench Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a browser-first PSO demo workbench that keeps one slide-friendly shell while letting the user switch between `Remotion`, `Motion Canvas`, and `Manim` implementations plus per-library layout variants of the same storyboard.

**Architecture:** Add a new React and Vite host app under `Docs/prototypes/workbench`, a shared storyboard module under `Docs/prototypes/shared`, and lightweight bridge layers for the existing `remotion-demo` and `motion-canvas-demo` packages. The host owns controls, notes, and the bottom progress bubbles, while each library surface responds to canonical `library`, `variant`, and `step` state through a small adapter contract.

**Tech Stack:** React, TypeScript, Vite, Vitest, Testing Library, existing Remotion and Motion Canvas packages, prerendered Manim assets, npm scripts.

---

### Task 1: Create the shared storyboard contract

**Files:**
- Create: `Docs/prototypes/shared/pso-workbench-types.ts`
- Create: `Docs/prototypes/shared/pso-storyboard.ts`
- Create: `Docs/prototypes/workbench/src/storyboard/pso-storyboard.test.ts`

**Step 1: Write the failing storyboard test**

```ts
import {describe, expect, it} from "vitest";
import {masterStoryboard} from "../../../shared/pso-storyboard";

describe("masterStoryboard", () => {
  it("locks the canonical step order", () => {
    expect(masterStoryboard.steps.map((step) => step.id)).toEqual([
      "base_formula",
      "open_fx",
      "inline_material",
      "shared_code",
    ]);
  });
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- pso-storyboard`
Expected: FAIL because the shared storyboard module does not exist yet.

**Step 3: Implement the shared types and master storyboard**

```ts
export type LibraryId = "remotion" | "motion-canvas" | "manim";

export const masterStoryboard = {
  storyId: "pso-concept",
  steps: [
    {id: "base_formula", label: "A -> f(x) -> B"},
    {id: "open_fx", label: "Open f(x)"},
    {id: "inline_material", label: "Inline material"},
    {id: "shared_code", label: "Shared code"},
  ],
} as const;
```

**Step 4: Run the test again**

Run: `npm --prefix Docs/prototypes/workbench run test -- pso-storyboard`
Expected: PASS with the canonical step list locked.

**Step 5: Commit**

```bash
git add Docs/prototypes/shared/pso-workbench-types.ts Docs/prototypes/shared/pso-storyboard.ts Docs/prototypes/workbench/src/storyboard/pso-storyboard.test.ts
git commit -m "feat: add shared PSO storyboard contract"
```

### Task 2: Scaffold the host workbench and lock shell behavior

**Files:**
- Create: `Docs/prototypes/workbench/package.json`
- Create: `Docs/prototypes/workbench/tsconfig.json`
- Create: `Docs/prototypes/workbench/vite.config.ts`
- Create: `Docs/prototypes/workbench/index.html`
- Create: `Docs/prototypes/workbench/src/main.tsx`
- Create: `Docs/prototypes/workbench/src/App.tsx`
- Create: `Docs/prototypes/workbench/src/app.css`
- Create: `Docs/prototypes/workbench/src/components/ControlBar.tsx`
- Create: `Docs/prototypes/workbench/src/components/NotesPanel.tsx`
- Create: `Docs/prototypes/workbench/src/components/ProgressBubbles.tsx`
- Create: `Docs/prototypes/workbench/src/components/StageFrame.tsx`
- Create: `Docs/prototypes/workbench/src/state/useWorkbenchState.ts`
- Create: `Docs/prototypes/workbench/src/App.test.tsx`

**Step 1: Write the failing host shell test**

```tsx
import {render, screen} from "@testing-library/react";
import {App} from "./App";

it("shows the current step and all progress bubbles", () => {
  render(<App />);
  expect(screen.getByText("A -> f(x) -> B")).toBeInTheDocument();
  expect(screen.getAllByRole("listitem")).toHaveLength(4);
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- App`
Expected: FAIL because the host app files do not exist yet.

**Step 3: Implement the minimal host shell**

```tsx
export function App() {
  const state = useWorkbenchState();

  return (
    <div className="workbench-shell">
      <ControlBar state={state} />
      <main className="workbench-main">
        <NotesPanel state={state} />
        <StageFrame state={state} />
      </main>
      <ProgressBubbles state={state} />
    </div>
  );
}
```

**Step 4: Run the test again**

Run: `npm --prefix Docs/prototypes/workbench run test -- App`
Expected: PASS with the shell rendering one notes panel and four bubbles.

**Step 5: Build the host shell**

Run: `npm --prefix Docs/prototypes/workbench run build`
Expected: PASS and emit a production build under `Docs/prototypes/workbench/dist`.

**Step 6: Commit**

```bash
git add Docs/prototypes/workbench
git commit -m "feat: scaffold PSO web workbench shell"
```

### Task 3: Add the library registry and fallback rules

**Files:**
- Create: `Docs/prototypes/workbench/src/libraries/types.ts`
- Create: `Docs/prototypes/workbench/src/libraries/registry.ts`
- Create: `Docs/prototypes/workbench/src/libraries/registry.test.ts`
- Modify: `Docs/prototypes/workbench/src/state/useWorkbenchState.ts`
- Modify: `Docs/prototypes/workbench/src/components/ControlBar.tsx`

**Step 1: Write the failing registry fallback test**

```ts
import {describe, expect, it} from "vitest";
import {resolveLibraryState} from "./registry";

describe("resolveLibraryState", () => {
  it("falls back to the library default variant", () => {
    const resolved = resolveLibraryState({
      libraryId: "manim",
      variantId: "bus-wide",
      stepId: "shared_code",
    });

    expect(resolved.variantId).toBe("reference");
  });
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- registry`
Expected: FAIL because the registry and fallback logic do not exist yet.

**Step 3: Implement the library metadata and resolver**

```ts
export const libraryRegistry = {
  remotion: {defaultVariantId: "bus-clean", steps: masterSteps},
  "motion-canvas": {defaultVariantId: "bus-clean", steps: masterSteps},
  manim: {defaultVariantId: "reference", steps: ["base_formula", "shared_code"]},
} as const;
```

**Step 4: Run the test again**

Run: `npm --prefix Docs/prototypes/workbench run test -- registry`
Expected: PASS with unsupported variants and steps falling back correctly.

**Step 5: Commit**

```bash
git add Docs/prototypes/workbench/src/libraries Docs/prototypes/workbench/src/state/useWorkbenchState.ts Docs/prototypes/workbench/src/components/ControlBar.tsx
git commit -m "feat: add workbench library registry"
```

### Task 4: Bridge the Remotion demo into the host

**Files:**
- Create: `Docs/prototypes/remotion-demo/src/embed.ts`
- Modify: `Docs/prototypes/remotion-demo/src/Root.tsx`
- Modify: `Docs/prototypes/remotion-demo/src/Composition.tsx`
- Create: `Docs/prototypes/workbench/src/adapters/remotionAdapter.ts`
- Create: `Docs/prototypes/workbench/src/adapters/remotionAdapter.test.ts`

**Step 1: Write the failing Remotion adapter test**

```ts
import {describe, expect, it} from "vitest";
import {createRemotionAdapter} from "./remotionAdapter";

describe("createRemotionAdapter", () => {
  it("builds an embed url with library, variant, and step", () => {
    const adapter = createRemotionAdapter("http://127.0.0.1:4174");
    expect(adapter.getEmbedUrl({
      libraryId: "remotion",
      variantId: "bus-clean",
      stepId: "open_fx",
    })).toContain("step=open_fx");
  });
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- remotionAdapter`
Expected: FAIL because the adapter does not exist yet.

**Step 3: Add embed mode to the Remotion app**

```ts
const state = readEmbedStateFromLocation(window.location.search);
window.parent.postMessage({type: "pso-workbench:ready"}, "*");
window.addEventListener("message", handleWorkbenchMessage);
```

**Step 4: Implement the host-side Remotion adapter**

```ts
export function createRemotionAdapter(baseUrl: string) {
  return {
    id: "remotion",
    getEmbedUrl(state) {
      const query = new URLSearchParams({
        embed: "1",
        variant: state.variantId,
        step: state.stepId,
      });
      return `${baseUrl}/?${query.toString()}`;
    },
  };
}
```

**Step 5: Run verification**

Run: `npm --prefix Docs/prototypes/workbench run test -- remotionAdapter`
Expected: PASS

Run: `npm --prefix Docs/prototypes/remotion-demo run lint`
Expected: PASS with the embed bridge compiling cleanly.

**Step 6: Commit**

```bash
git add Docs/prototypes/remotion-demo/src Docs/prototypes/workbench/src/adapters/remotionAdapter.ts Docs/prototypes/workbench/src/adapters/remotionAdapter.test.ts
git commit -m "feat: connect Remotion workbench bridge"
```

### Task 5: Bridge the Motion Canvas demo into the host

**Files:**
- Create: `Docs/prototypes/motion-canvas-demo/src/embed.ts`
- Modify: `Docs/prototypes/motion-canvas-demo/src/project.ts`
- Modify: `Docs/prototypes/motion-canvas-demo/src/scenes/example.tsx`
- Create: `Docs/prototypes/workbench/src/adapters/motionCanvasAdapter.ts`
- Create: `Docs/prototypes/workbench/src/adapters/motionCanvasAdapter.test.ts`

**Step 1: Write the failing Motion Canvas adapter test**

```ts
import {describe, expect, it} from "vitest";
import {createMotionCanvasAdapter} from "./motionCanvasAdapter";

describe("createMotionCanvasAdapter", () => {
  it("targets the Motion Canvas embed surface", () => {
    const adapter = createMotionCanvasAdapter("http://127.0.0.1:4175");
    expect(adapter.getEmbedUrl({
      libraryId: "motion-canvas",
      variantId: "bus-clean",
      stepId: "shared_code",
    })).toContain("variant=bus-clean");
  });
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- motionCanvasAdapter`
Expected: FAIL because the adapter does not exist yet.

**Step 3: Add embed mode to Motion Canvas**

```ts
export default makeProject({
  scenes: [example],
  variables: {
    variant: "bus-clean",
    step: "base_formula",
  },
});
```

**Step 4: Implement the host-side Motion Canvas adapter**

```ts
export function createMotionCanvasAdapter(baseUrl: string) {
  return {
    id: "motion-canvas",
    getEmbedUrl(state) {
      const query = new URLSearchParams({
        embed: "1",
        variant: state.variantId,
        step: state.stepId,
      });
      return `${baseUrl}/?${query.toString()}`;
    },
  };
}
```

**Step 5: Run verification**

Run: `npm --prefix Docs/prototypes/workbench run test -- motionCanvasAdapter`
Expected: PASS

Run: `npm --prefix Docs/prototypes/motion-canvas-demo run build`
Expected: PASS with the bridge-aware project building successfully.

**Step 6: Commit**

```bash
git add Docs/prototypes/motion-canvas-demo/src Docs/prototypes/workbench/src/adapters/motionCanvasAdapter.ts Docs/prototypes/workbench/src/adapters/motionCanvasAdapter.test.ts
git commit -m "feat: connect Motion Canvas workbench bridge"
```

### Task 6: Add the Manim reference adapter and offline states

**Files:**
- Create: `Docs/prototypes/workbench/src/adapters/manimAdapter.ts`
- Create: `Docs/prototypes/workbench/src/adapters/manimAdapter.test.ts`
- Modify: `Docs/prototypes/workbench/src/components/StageFrame.tsx`
- Modify: `Docs/prototypes/workbench/src/components/NotesPanel.tsx`

**Step 1: Write the failing Manim adapter test**

```ts
import {describe, expect, it} from "vitest";
import {createManimAdapter} from "./manimAdapter";

describe("createManimAdapter", () => {
  it("maps canonical steps to reference media timestamps", () => {
    const adapter = createManimAdapter();
    expect(adapter.resolveStep("shared_code").timeInSeconds).toBeGreaterThan(0);
  });
});
```

**Step 2: Run the test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- manimAdapter`
Expected: FAIL because the adapter does not exist yet.

**Step 3: Implement the reference-media adapter**

```ts
const MANIM_STEP_MAP = {
  base_formula: {timeInSeconds: 0},
  open_fx: {timeInSeconds: 0.8},
  inline_material: {timeInSeconds: 1.6},
  shared_code: {timeInSeconds: 2.2},
};
```

**Step 4: Add offline and unsupported-state UI to the host**

```tsx
if (adapter.kind === "reference") {
  return <video src={adapter.videoUrl} poster={adapter.posterUrl} />;
}
```

**Step 5: Run verification**

Run: `npm --prefix Docs/prototypes/workbench run test -- manimAdapter`
Expected: PASS

Run: `npm --prefix Docs/prototypes/workbench run build`
Expected: PASS with the reference adapter bundled correctly.

**Step 6: Commit**

```bash
git add Docs/prototypes/workbench/src/adapters/manimAdapter.ts Docs/prototypes/workbench/src/adapters/manimAdapter.test.ts Docs/prototypes/workbench/src/components/StageFrame.tsx Docs/prototypes/workbench/src/components/NotesPanel.tsx
git commit -m "feat: add Manim comparison adapter"
```

### Task 7: Add one-command startup and final verification

**Files:**
- Create: `Docs/prototypes/package.json`
- Modify: `Docs/prototypes/README.md`
- Create: `Docs/prototypes/workbench/playwright.config.ts`
- Create: `Docs/prototypes/workbench/tests/workbench-smoke.spec.ts`

**Step 1: Write the failing smoke test**

```ts
import {expect, test} from "@playwright/test";

test("switches libraries from the host shell", async ({page}) => {
  await page.goto("http://127.0.0.1:4173");
  await page.getByLabel("Library").selectOption("motion-canvas");
  await expect(page.getByText("Shared code")).toBeVisible();
});
```

**Step 2: Run the smoke test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test:e2e`
Expected: FAIL because the orchestrated local stack does not exist yet.

**Step 3: Add orchestration scripts**

```json
{
  "scripts": {
    "dev:workbench": "npm --prefix workbench run dev",
    "dev:remotion": "npm --prefix remotion-demo run dev -- --host 127.0.0.1 --port 4174",
    "dev:motion": "npm --prefix motion-canvas-demo run start -- --host 127.0.0.1 --port 4175",
    "dev": "concurrently -k \"npm:dev:workbench\" \"npm:dev:remotion\" \"npm:dev:motion\""
  }
}
```

**Step 4: Run the final verification suite**

Run: `npm --prefix Docs/prototypes run dev`
Expected: Host plus child surfaces come up on their assigned ports.

Run: `npm --prefix Docs/prototypes/workbench run test`
Expected: PASS

Run: `npm --prefix Docs/prototypes/workbench run test:e2e`
Expected: PASS

Run: `npm --prefix Docs/prototypes/workbench run build`
Expected: PASS

**Step 5: Commit**

```bash
git add Docs/prototypes/package.json Docs/prototypes/README.md Docs/prototypes/workbench/playwright.config.ts Docs/prototypes/workbench/tests/workbench-smoke.spec.ts
git commit -m "feat: add PSO workbench dev orchestration"
```
