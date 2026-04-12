# SlideApp Review Mode And Skill Update Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a browser-first review mode to `SlideApp` so a target page can be opened instantly, captured as a stage-only screenshot, and scored locally before more diagram edits; update the diagram-review skill to require this loop.

**Architecture:** Keep the existing workbench intact and layer a small `review` workflow on top: URL-driven initial state, a compact review HUD in the app shell, and a second capture target bound to the stage only. In parallel, update the diagram-review skill docs so rendered browser screenshots, page-jump controls, and a numeric self-critique loop become mandatory before declaring a page acceptable.

**Tech Stack:** React, Vitest, existing `html-to-image` capture utility, Markdown skill docs.

---

### Task 1: Lock query-driven review startup in tests

**Files:**
- Modify: `SlideApp/src/App.test.tsx`

**Step 1: Write failing tests**

- Add a test that boots `App` from `?step=page_08&controls=1&review=1&motion=half`.
- Add a test that clicks a review HUD step selector and lands on the requested page.
- Add a test that uses a review HUD capture button and verifies `toBlob` targets the stage-only element, not `.workbench-shell`.

**Step 2: Run targeted tests to verify failure**

Run: `cd SlideApp && npm test -- src/App.test.tsx -t "review mode|stage-only"`

Expected: FAIL because review-mode UI and stage-only capture do not exist yet.

### Task 2: Add review-mode state plumbing

**Files:**
- Modify: `SlideApp/src/state/useWorkbenchState.ts`
- Modify: `SlideApp/src/App.tsx`

**Step 1: Parse initial URL params**

- Support `step`, `variant`, `motion`, `controls`, and `review` on first render.
- Feed initial `step` / `variant` into `useWorkbenchState`.

**Step 2: Sync live state back into the URL**

- Use `history.replaceState` so the current page can be copied / reopened directly.

**Step 3: Re-run targeted tests**

Run: `cd SlideApp && npm test -- src/App.test.tsx -t "review mode|query"`

Expected: startup/query tests pass or move to the next missing UI failure.

### Task 3: Add the compact review HUD and stage-only capture

**Files:**
- Create: `SlideApp/src/components/ReviewHud.tsx`
- Modify: `SlideApp/src/components/CaptureClipboardButton.tsx`
- Modify: `SlideApp/src/components/StageFrame.tsx`
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/app.css`

**Step 1: Add a second capture target**

- Mark the stage container as a dedicated capture surface.
- Reuse the existing capture logic for an inline review HUD button.

**Step 2: Build the HUD**

- Add quick prev/next navigation.
- Add a step dropdown for direct jumps.
- Add a stage-only capture button.
- Add four local review scores: `Blocker`, `Routing`, `Balance`, `Focus`.
- Show an overall score and a simple gate message when the average is below target.

**Step 3: Style the HUD**

- Keep it small, translucent, and bottom-right.
- Do not steal emphasis from the main stage.

**Step 4: Run targeted app tests**

Run: `cd SlideApp && npm test -- src/App.test.tsx -t "review mode|stage-only"`

Expected: PASS.

### Task 4: Update the diagram-review skill contract

**Files:**
- Modify: `Docs/skills/drawio-tech-diagram-review/SKILL.md`

**Step 1: Add a browser-review loop**

- Require jumping directly to the target page before review.
- Require a rendered browser screenshot, preferably stage-only.
- Require a four-axis self-score before claiming success.
- Require rework when the score or blocker scan is below threshold.

**Step 2: Add SlideApp-specific hooks**

- Mention URL / HUD page jumps and stage-only capture for this project.

### Task 5: Verify end-to-end

**Files:**
- Modify: none unless verification finds issues

**Step 1: Run the full test suite**

Run: `cd SlideApp && npm test`

Expected: PASS.

**Step 2: Run the production build**

Run: `cd SlideApp && npm run build`

Expected: PASS.
