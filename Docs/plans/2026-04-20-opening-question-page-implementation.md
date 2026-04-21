# Opening Question Page Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add one new opening page before `page_01` that reuses the runtime stutter evidence image plus the two platform-delta sample images, and frames the talk as six questions that will be answered later.

**Architecture:** Introduce a new storyboard step `page_00` instead of overloading the existing `page_01`. Keep the current minimal-model page intact, add a dedicated Remotion scene for the new opening layout, and retime only the first anchor window so the rest of the deck changes as little as possible.

**Tech Stack:** TypeScript, React, Remotion, Vitest, existing storyboard metadata, Python slide harness audits

---

### Task 1: Add `page_00` to storyboard and step metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/state/useWorkbenchState.ts`
- Test: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Extend the step id union**

- Add `page_00` to `StoryStepId`.
- Keep the existing step ids and naming pattern unchanged.

**Step 2: Insert the new step into session 1 and the canonical step order**

- Put `page_00` before `page_01`.
- Keep it in session 1 so the opening still belongs to the foundation chapter.

**Step 3: Write the opening-page storyboard copy**

- Add a new `page_00` entry to `masterStoryboard.steps`.
- Reuse the approved question-driven copy:
  - left pain point from `pso-stutter.png`
  - bottom anonymous sample A / B images
  - six questions only, no full answers
- Update the global storyboard summary text so it mentions the new opening question page.

**Step 4: Make the workbench open on the new first page**

- Change `DEFAULT_STEP_ID` from `page_01` to `page_00`.
- Keep all other default behavior unchanged.

**Step 5: Run the focused storyboard test**

Run: `./node_modules/.bin/vitest run src/storyboard/pso-storyboard.test.ts`
Expected: PASS, with the canonical step order and `page_00` copy updated.

### Task 2: Add the new timeline anchor without disturbing the rest of the deck more than necessary

**Files:**
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`
- Test: `SlideApp/src/remotion/sceneTimeline.test.ts`

**Step 1: Add a new Remotion step anchor for `page_00`**

- Introduce `page_00` into `STEP_FRAME_MAP`.
- Give `page_00` the old first-slot window and move `page_01` to the next anchor so `page_00 -> page_01` has a real transition budget.
- Avoid a full-deck retime if the first-window-only shift is sufficient.

**Step 2: Insert `page_00` into the canonical Remotion sequence**

- Add it as the first entry of `REMOTION_STEP_SEQUENCE`.
- Keep all later order relationships intact.

**Step 3: Export the new page-frame constant**

- Add `PAGE_00_FRAME` to `page-layout-constants.ts` if the new scene logic needs it.
- Keep `PAGE_01_FRAME` and later constants working exactly as before.

**Step 4: Run the timeline test**

Run: `./node_modules/.bin/vitest run src/remotion/sceneTimeline.test.ts`
Expected: PASS, with `page_00` as the first step and updated first-window expectations.

### Task 3: Build the new opening page scene

**Files:**
- Create: `SlideApp/src/remotion/pages/Page00OpeningScene.tsx`
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/remotion/publicAssetPath.ts` only if a new helper export is genuinely needed
- Test: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Create the opening-page layout component**

- Build a new scene component that renders:
  - one large left evidence image using `/supplement/pso-stutter.png`
  - one right question card with six bold questions
  - two small bottom sample images using:
    - `/supplement/ogl-mtl/opengl-compile-count.png`
    - `/supplement/ogl-mtl/ios-compile-count.png`
- Keep the deck background language consistent with the current project.

**Step 2: Make the opening page anonymous but readable**

- Label the two small images as `样本 A` / `样本 B`.
- Do not expose platform names on this page.
- Keep the question list scannable at presentation distance.

**Step 3: Hook the new scene into the composition**

- Render the new scene as an overlay that is visible during the `page_00` window.
- Fade it out as `page_01` takes over.
- Do not disturb the existing page 01–33 scene components beyond what the new overlay requires.

**Step 4: Write focused composition tests**

- Add one targeted test that renders the `page_00` frame and asserts:
  - the stutter image is visible
  - both sample images are visible
  - the question list contains representative opening questions
- Add one targeted test around the first handoff frame to confirm the opening page disappears and `page_01` becomes visible.

**Step 5: Run the targeted composition tests**

Run: `./node_modules/.bin/vitest run src/remotion/Composition.test.tsx -t "page 00"`
Expected: PASS for the new opening-page tests.

### Task 4: Sync app-shell expectations for the new first page

**Files:**
- Modify: `SlideApp/src/App.test.tsx`
- Modify: `SlideApp/src/progressRailSessionColors.test.tsx` only if the new first step affects explicit expectations
- Modify: `SlideApp/src/components/NotesPanel.test.tsx` only if the new default step or notes flow breaks assumptions

**Step 1: Update default-step expectations**

- Fix any tests that assume the app opens on `page_01`.
- Keep the user-visible step ordering aligned with the new storyboard order.

**Step 2: Check session-rail assumptions**

- Ensure `page_00` belongs to session 1.
- Update tests only where the explicit first-step assumption has changed.

**Step 3: Run the most relevant app-shell tests**

Run: `./node_modules/.bin/vitest run src/App.test.tsx src/progressRailSessionColors.test.tsx src/components/NotesPanel.test.tsx`
Expected: PASS or only targeted failures caused by the new first-step behavior.

### Task 5: Write and sync the page script docs

**Files:**
- Create: `Docs/剧本/00-开场问题页.md`
- Modify: `Docs/剧本/01-第一页.md`

**Step 1: Write the new opening-page script**

- Add the standard sections already used in other page docs:
  - page goal
  - page change table
  - node / edge or layout inventory as appropriate
  - animation script
  - remotion notes
- Explicitly record the reused images and the six questions.

**Step 2: Update page 01 continuity**

- Change `page_01` so its previous page is the new opening page.
- Keep page 01’s own semantic role as the minimal model.

**Step 3: Record the new transition expectation**

- Document that `page_00 -> page_01` is a whole-page fade handoff, not a structural morph.
- Keep the wording aligned with the repo’s image/explanation-page transition rules.

### Task 6: Run harness audits and final verification

**Files:**
- Modify if needed: `Docs/剧本/00-剧本与页面对齐审计.md`

**Step 1: Run transition timing audit**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
Expected: PASS, with the new opening-page transition included in the timing ledger.

**Step 2: Run storyboard sync audit**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
Expected: PASS, and refresh the generated sync-audit doc if the tool updates it.

**Step 3: Run mechanical review from page 19**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`
Expected: PASS with no blocker pages.

**Step 4: Run a final focused test bundle**

Run: `./node_modules/.bin/vitest run src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts`
Expected: PASS

### Task 7: Final review and commit

**Files:**
- Stage only files changed for the opening-page feature

**Step 1: Inspect the final diff**

Run: `git status --short`
Expected: only intended opening-page files plus any pre-existing unrelated edits already in the workspace.

**Step 2: Commit the feature**

```bash
git add Docs/plans/2026-04-20-opening-question-page-design.md \
        Docs/plans/2026-04-20-opening-question-page-implementation.md \
        Docs/剧本/00-开场问题页.md \
        Docs/剧本/01-第一页.md \
        SlideApp/src/storyboard-data/pso-workbench-types.ts \
        SlideApp/src/storyboard-data/pso-storyboard.ts \
        SlideApp/src/state/useWorkbenchState.ts \
        SlideApp/src/remotion/embed.ts \
        SlideApp/src/remotion/sceneTimeline.ts \
        SlideApp/src/remotion/pages/page-layout-constants.ts \
        SlideApp/src/remotion/pages/Page00OpeningScene.tsx \
        SlideApp/src/remotion/Composition.tsx
git commit -m "feat: add opening question page"
```

Expected: commit succeeds without staging unrelated files.
