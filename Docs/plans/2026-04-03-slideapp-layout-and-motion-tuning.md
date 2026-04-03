# SlideApp Layout And Motion Tuning Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Tune the local SlideApp presentation layout and transition pacing without deploying, making the notes panel larger, the stage slightly smaller, and both motion systems calmer at the baseline while keeping the visible default motion option at `1x`.

**Architecture:** Keep the current split-screen composition and expose its balance through shell-level CSS custom properties controlled from `App.tsx`. Remove the redundant step counter from the notes card header and retune both the notes-card exit animation and progress rail timing in CSS so the interface feels slower and more deliberate.

**Tech Stack:** React, TypeScript, CSS, Vitest, Testing Library.

---

### Task 1: Lock the intended defaults in tests

**Files:**
- Modify: `SlideApp/src/App.test.tsx`
- Modify: `SlideApp/src/appStyles.test.ts`

**Step 1:** Add expectations for the default motion preset, the absence of `Step x / y` copy, and the presence of shell-level layout tuning variables.

**Step 2:** Add style assertions for the widened notes column layout rule and the slower, ease-in card pull-away curve.

### Task 2: Implement the layout and copy changes

**Files:**
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/components/NotesPanel.tsx`
- Modify: `SlideApp/src/app.css`

**Step 1:** Introduce adjustable layout constants in `App.tsx` and pass them to the shell as CSS custom properties.

**Step 2:** Remove the `Step x / y` kicker from the notes card header while preserving the focus chip and note hierarchy.

**Step 3:** Update the main split layout to give the notes panel more width and slightly reduce the stage column share.

### Task 3: Retune motion defaults

**Files:**
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/app.css`

**Step 1:** Keep the default motion preset at `1x`, but slow the underlying notes-motion baseline.

**Step 2:** Retune the rail timing factor to `0.5`.

**Step 3:** Change the notes-card exit easing so it starts slower and accelerates later in the movement.

### Task 4: Verify locally without deployment

**Files:**
- Test: `SlideApp/src/App.test.tsx`
- Test: `SlideApp/src/appStyles.test.ts`

**Step 1:** Run targeted tests.

**Step 2:** Run the full SlideApp test suite.

**Step 3:** Run the SlideApp production build.
