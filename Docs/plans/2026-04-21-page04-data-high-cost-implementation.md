# Page 04 Data High-Cost Markers Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add visible high-cost markers to the three expensive `Avg` entries on `page_04_data`, and annotate `glCompileShader` as `x2` with doubled average values because both VS and FS are compiled.

**Architecture:** Keep the existing `page_04_data` table geometry unchanged. Extend the row metadata in `Page04DataScene.tsx` with stable keys plus optional `avg` highlight and `x2` annotation, then render small red markers inside the `Avg` column and a compact `x2` note inside the API column. Lock the behavior with a composition-level render test and sync storyboard/script wording so visual semantics and narration stay aligned.

**Tech Stack:** React 19, Remotion SVG scene rendering, Vitest + Testing Library, markdown storyboard docs.

---

### Task 1: Lock the desired page_04_data behavior with tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

- Add a test around `page_04_data` that expects:
  - exactly three visible high-cost markers on the `Avg` column
  - a visible `x2` annotation for `glCompileShader`
  - the doubled `Avg` value `7.262 / 15.350`

**Step 2: Run test to verify it fails**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "highlights the expensive avg rows on page 04 data and annotates glCompileShader as x2"`

Expected: FAIL because the marker test ids and updated text do not exist yet.

### Task 2: Implement the minimal page_04_data rendering change

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page04DataScene.tsx`

**Step 1: Extend row metadata**

- Add a stable row key for each data row.
- Mark `glCompileShader`, `glLinkProgram`, and `CreateGfxPipeline` as high-cost `Avg` rows.
- Update `glCompileShader.avg` to `7.262 / 15.350`.
- Add a compact `x2` annotation for the compile row.

**Step 2: Render the markers**

- Keep the existing table width, column widths, and row heights unchanged.
- Render a small red `HIGH` marker near the right edge of the `Avg` column for the three highlighted rows.
- Render a small red `x2` note in the API column for `glCompileShader`.

**Step 3: Run the targeted test**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "highlights the expensive avg rows on page 04 data and annotates glCompileShader as x2"`

Expected: PASS.

### Task 3: Sync docs and storyboard wording

**Files:**
- Modify: `Docs/剧本/05-第五页.md`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Test: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Update script wording**

- Mention that the data page now gives the three expensive `Avg` rows explicit marks.
- Mention that `glCompileShader` is shown as `x2`, because VS and FS both compile.

**Step 2: Add or tighten storyboard assertions if needed**

- Keep the test focused on manuscript text only.

### Task 4: Verify the full required closure

**Files:**
- No additional source changes expected

**Step 1: Run focused tests**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "inserts a dedicated data checkpoint page between page 04 and page 05|highlights the expensive avg rows on page 04 data and annotates glCompileShader as x2"`

Expected: PASS.

**Step 2: Run storyboard tests**

Run: `npm --silent --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`

Expected: PASS.

**Step 3: Run formal-page mechanical review**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

Expected: no blocker pages introduced by this change.

**Step 4: Run harness audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Expected: audit artifacts generated successfully.

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: storyboard sync audit completes successfully.
