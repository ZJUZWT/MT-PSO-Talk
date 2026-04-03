# PSO Storyboard API Expansion Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Expand the PSO web workbench into a six-step teaching sequence that covers the abstract model, OpenGL, Vulkan, and the existing PSO breakdown while fixing the current line, arrow, and focus-color issues.

**Architecture:** Extend the shared storyboard contract to carry richer lecture-panel content and two new storyboard steps. Rework the Remotion composition to support page-specific layouts with one focus color per page, then update the left panel so it can show speaker copy and code snippets that mirror the diagram semantics. Verify in both Chromium and WebKit because the workbench is slide-facing and browser consistency matters.

**Tech Stack:** React, Remotion, TypeScript, Vitest, Vite, Playwright.

---

### Task 1: Expand the storyboard types for a lecture-oriented left panel

**Files:**
- Modify: `Docs/prototypes/shared/pso-workbench-types.ts`
- Modify: `Docs/prototypes/shared/pso-storyboard.ts`
- Test: `Docs/prototypes/workbench/src/storyboard/pso-storyboard.test.ts`

**Step 1: Write the failing test**

Add assertions that:

- the storyboard exposes 6 ordered steps
- the new `opengl_state_machine` and `vulkan_pso` steps exist
- at least one step carries code-block content metadata

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: FAIL until the shared types and storyboard data are expanded.

**Step 3: Write minimal implementation**

- Add two new step ids for OpenGL and Vulkan
- Extend the step type with optional lecture content fields:
  - `intro`
  - `speakerNotes`
  - `codeSample`
  - `codeLegend`
  - `focusColorKey`
- Populate the six-step sequence in `pso-storyboard.ts`

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: PASS.

### Task 2: Rebuild the left panel as a lecture panel

**Files:**
- Modify: `Docs/prototypes/workbench/src/components/NotesPanel.tsx`
- Modify: `Docs/prototypes/workbench/src/app.css`
- Test: `Docs/prototypes/workbench/src/App.test.tsx`

**Step 1: Write the failing test**

Add an assertion that the panel can render:

- the page intro
- the speaker note
- an optional code block when the current step provides one

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: FAIL until the new lecture panel structure exists.

**Step 3: Write minimal implementation**

- Replace the current note-only layout with:
  - kicker
  - title
  - manuscript-style intro paragraph
  - speaker-note or narration block
  - optional code block
  - optional legend row
- Style the code block like slide content, not editor chrome
- Keep the panel height aligned with the right canvas

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: PASS.

### Task 3: Add a collapsible top control rail and centered step navigator

**Files:**
- Modify: `Docs/prototypes/workbench/src/App.tsx`
- Modify: `Docs/prototypes/workbench/src/components/ControlBar.tsx`
- Modify: `Docs/prototypes/workbench/src/components/ProgressBubbles.tsx`
- Modify: `Docs/prototypes/workbench/src/state/useWorkbenchState.ts`
- Modify: `Docs/prototypes/workbench/src/app.css`
- Test: `Docs/prototypes/workbench/src/App.test.tsx`

**Step 1: Write the failing test**

Add assertions that:

- the top controls render collapsed by default
- the user can expand them on demand
- the current step renders as the expanded center item in the bottom rail

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: FAIL until the new presentation-first shell exists.

**Step 3: Write minimal implementation**

- Add a collapsed-by-default top control rail
- Preserve the existing selects inside the expanded state
- Rebuild the bottom progress area as a centered storyboard rail:
  - inactive items compact
  - active item expanded
  - active item visually centered when possible

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: PASS.

### Task 4: Lock the new diagram geometry in tests

**Files:**
- Modify: `Docs/prototypes/shared/diagramLayout.ts`
- Modify: `Docs/prototypes/workbench/src/storyboard/diagramLayout.test.ts`

**Step 1: Write the failing test**

Add assertions that:

- the stage panel remains near full-bleed on the right
- inline material connectors all terminate with explicit arrow targets
- the shared-code layout supports a staggered or stacked source arrangement
- default non-focus links stay on the neutral wire system

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/diagramLayout.test.ts`

Expected: FAIL until the new geometry contract is encoded.

**Step 3: Write minimal implementation**

- Add layout values for the two new API pages
- Add positions for stacked-card material sources
- Add neutral/focus routing anchor points for inline and shared pages

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/diagramLayout.test.ts`

Expected: PASS.

### Task 5: Rebuild the Remotion composition for the six-step story

**Files:**
- Modify: `Docs/prototypes/remotion-demo/src/Composition.tsx`
- Modify: `Docs/prototypes/remotion-demo/src/embed.ts`
- Modify: `Docs/prototypes/shared/playbackTimeline.ts`
- Test: `Docs/prototypes/workbench/src/storyboard/playbackTimeline.test.ts`

**Step 1: Write the failing test**

Add assertions that the playback timeline resolves frames for all six steps in
story order.

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/playbackTimeline.test.ts`

Expected: FAIL until the new steps exist in the timeline.

**Step 3: Write minimal implementation**

- Add frame anchors for the two inserted API pages
- Replace the one-layout-fits-all drawing logic with per-step rendering branches
- Implement:
  - abstract model page
  - OpenGL fragmented-state page
  - Vulkan unified-PSO page
  - PSO open page
  - inline-material page with explicit arrows
  - shared-code page with stacked-card source convergence

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/storyboard/playbackTimeline.test.ts`

Expected: PASS.

### Task 6: Normalize focus color behavior

**Files:**
- Modify: `Docs/prototypes/remotion-demo/src/Composition.tsx`
- Modify: `Docs/prototypes/workbench/src/components/NotesPanel.tsx`
- Test: `Docs/prototypes/workbench/src/App.test.tsx`

**Step 1: Write the failing test**

Add a lightweight rendering contract assertion that the current step exposes a
single `focusTarget` and a single `focusColorKey` for the lecture panel.

**Step 2: Run test to verify it fails**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: FAIL until the data contract and panel rendering support this.

**Step 3: Write minimal implementation**

- Ensure the current page focus maps cleanly to one orange-highlight concept
- Make default connectors neutral gray
- Match lecture-panel legend chips to the active concept mapping

**Step 4: Run test to verify it passes**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx`

Expected: PASS.

### Task 7: Verify browser consistency and capture slide artifacts

**Files:**
- Output: `Docs/prototypes/.smoke-artifacts/pso-storyboard-api-expansion/`

**Step 1: Run targeted tests**

Run: `npm --prefix Docs/prototypes/workbench run test -- --run src/App.test.tsx src/storyboard/diagramLayout.test.ts src/storyboard/pso-storyboard.test.ts src/storyboard/playbackTimeline.test.ts`

Expected: PASS.

**Step 2: Run production build**

Run: `npm --prefix Docs/prototypes run build`

Expected: PASS.

**Step 3: Run browser smoke**

Use Playwright against `http://127.0.0.1:4173` and capture Chromium + WebKit
screenshots for:

- abstract model
- OpenGL page
- Vulkan page
- inline material
- shared shader code

Verify:

- right stage fills the available panel area
- top controls render identically across engines
- inline-material page has visible arrowheads
- shared-code page uses stacked convergence with a single path into
  `ShaderCodeLib`
- each page has only one orange conceptual focus

**Step 4: Save artifacts**

Save screenshots under:

- `Docs/prototypes/.smoke-artifacts/pso-storyboard-api-expansion/chromium-*.png`
- `Docs/prototypes/.smoke-artifacts/pso-storyboard-api-expansion/webkit-*.png`
