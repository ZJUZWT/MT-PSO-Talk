# Session Colored Progress Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add session-based color grouping to the bottom progress rail while preserving the existing current-step highlight hierarchy.

**Architecture:** Derive a session token for each step by looking up the step inside the existing storyboard sessions. Feed that token into the progress bubble DOM as a data attribute and drive the colors in CSS from a small set of session-scoped custom properties. Keep current/past/future behavior intact and only layer session tinting on top.

**Tech Stack:** React, TypeScript, CSS, Vitest

---

### Task 1: Write the failing regression tests

**Files:**
- Create: `SlideApp/src/progressRailSessionColors.test.tsx`
- Modify: `SlideApp/src/appStyles.test.ts`

**Step 1: Write the failing component test**

- Render `ProgressBubbles` with the real storyboard state.
- Assert compact bubbles in different sessions expose different session identifiers.
- Assert the current expanded bubble also exposes a session identifier.

**Step 2: Write the failing CSS test**

- Assert CSS has session-scoped progress bubble rules or variables.
- Assert compact dots use session color tokens.
- Assert expanded/current states still have dedicated styling on top.

**Step 3: Run tests to verify failure**

Run:
- `npm --prefix SlideApp run test -- src/progressRailSessionColors.test.tsx`
- `npm --prefix SlideApp run test -- src/appStyles.test.ts`

Expected: FAIL because progress bubbles do not yet encode or style session colors.

### Task 2: Add session lookup in ProgressBubbles

**Files:**
- Modify: `SlideApp/src/components/ProgressBubbles.tsx`

**Step 1: Derive session identity**

- Resolve each step’s parent session from `state.sessions`.
- Fall back to a neutral token if a step is somehow ungrouped.

**Step 2: Expose it to the DOM**

- Add a `data-session-color` or equivalent attribute on each shell/bubble.
- Keep current click behavior, sizing, and transition logic unchanged.

### Task 3: Add the session color system in CSS

**Files:**
- Modify: `SlideApp/src/app.css`

**Step 1: Define session palettes**

- Add 6 subtle session palettes as custom properties.
- Keep them distinct enough for adjacent groups to separate visually.

**Step 2: Style compact dots**

- Apply the session color primarily to compact dots.
- Preserve dot size and motion behavior.

**Step 3: Style expanded pills lightly**

- Add a light session-tinted border/background treatment for expanded states.
- Preserve the stronger current-state lift/glow so “current page” still wins visually.

### Task 4: Verify and document

**Files:**
- Optionally modify: `Docs/剧本/README.md` or relevant UI notes only if needed

**Step 1: Run focused tests**

Run:
- `npm --prefix SlideApp run test -- src/progressRailSessionColors.test.tsx`
- `npm --prefix SlideApp run test -- src/appStyles.test.ts`

Expected: PASS

**Step 2: Run build**

Run: `npm --prefix SlideApp run build`

Expected: PASS
