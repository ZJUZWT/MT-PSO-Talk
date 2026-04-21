# Page16 Notes StableKey Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a page16-only ShaderStableKey sample table to the left notes panel using the real MyMaterial SHK CSV row.

**Architecture:** Extend the storyboard step model with an optional notes-side key/value table payload, populate it only for `page_16`, and render it in `NotesPanel` as a dedicated section below the standard narrative blocks. Keep the slide canvas unchanged so the change stays isolated to the notes column.

**Tech Stack:** React, TypeScript, Vitest, existing storyboard data model

---

### Task 1: Add the failing notes-panel test

**Files:**
- Create: `SlideApp/src/components/NotesPanel.test.tsx`
- Read: `SlideApp/src/components/NotesPanel.tsx`
- Read: `SlideApp/src/storyboard-data/pso-storyboard.ts`

**Step 1: Write the failing test**

- Render `NotesPanel` with `page_16`.
- Assert the panel shows a `ShaderStableKey` sample section.
- Assert it contains the real `Asset` path and representative fields like `ShaderType`, `VFType`, and `OutputHash`.
- Assert the same section is absent for a nearby non-page16 step such as `page_15`.

**Step 2: Run test to verify it fails**

Run: `npm --prefix SlideApp run test -- src/components/NotesPanel.test.tsx`

Expected: FAIL because the notes panel does not yet support structured field tables.

### Task 2: Add structured notes table data to the storyboard model

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Test: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Extend the type**

- Add an optional `notesDataTable` field on `StoryStep`.
- Model it as a section title plus ordered rows of `{label, value}`.

**Step 2: Populate page16**

- Add a `notesDataTable` payload only to `page_16`.
- Use the real `MyMaterial` row from the SHK CSV as literal data in storyboard metadata.
- Preserve all 13 CSV fields in the row order used by the source file.

**Step 3: Add/update storyboard assertions**

- Assert `page_16.notesDataTable` exists and includes the `Asset` path.
- Assert a neighboring step like `page_15` has no such table.

### Task 3: Render the new section in NotesPanel

**Files:**
- Modify: `SlideApp/src/components/NotesPanel.tsx`

**Step 1: Add the section**

- Render a new notes section when `step.notesDataTable` exists.
- Use a simple two-column semantic table/list layout that wraps long values like `Asset`.

**Step 2: Keep layout scoped**

- Do not disturb existing sections for other pages.
- Keep the table visually secondary to the normal caption and key points.

### Task 4: Verify and document

**Files:**
- Modify: `Docs/剧本/16-第十六页-正式动画.md`

**Step 1: Run focused tests**

Run: `npm --prefix SlideApp run test -- src/components/NotesPanel.test.tsx src/storyboard/pso-storyboard.test.ts`

Expected: PASS for the newly affected units.

**Step 2: Update page16 script notes**

- Record that the left notes panel now carries the real `MyMaterial` `ShaderStableKey` full-field sample.
- Explicitly note that the slide canvas geometry and transition timing remain unchanged.

**Step 3: Optional wider verification**

Run: `npm --prefix SlideApp run test -- src/components/NotesPanel.test.tsx src/storyboard/pso-storyboard.test.ts src/remotion/Composition.test.tsx`

Expected: the new focused tests pass; if existing composition regressions still fail, report them as pre-existing or unrelated unless this change adds new failures.
