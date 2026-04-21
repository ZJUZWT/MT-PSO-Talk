# Page09img Material Instance Note Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Update `page_09_img` so the Material/MaterialInstance explanation sits outside above the comparison cards and reflects the real sample setup.

**Architecture:** Keep the existing evidence page structure intact, change only the explanatory copy placement, and sync the same story fact into the storyboard and page script. Protect the change with one targeted composition test, then verify with harness audits and a fresh browser-api capture.

**Tech Stack:** React SVG scene code, Vitest, SlideApp harness scripts, Markdown page script docs

---

### Task 1: Lock the new note behavior with a failing test

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing test**

Add one test that checks:
- the new outside legend line exists
- the two Static Bool footnote lines exist
- all three lines sit above the top card row
- the old in-card helper line is gone

**Step 2: Run test to verify it fails**

Run: `npm --prefix SlideApp test -- --run src/remotion/Composition.test.tsx -t "moves the page 09_img Material and instance explanation into an outside note above the comparison cards"`
Expected: FAIL because the old in-card helper line is still rendered.

### Task 2: Implement the new outside note in the page renderer

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Add the outside note**

Render one centered legend line and two smaller centered footnote lines above the top cards in `Page09EvidenceNotes(...)`.

**Step 2: Remove the old in-card helper**

Delete the old center-card helper line so the `.uexp` comparison card only carries the card title and table rows.

**Step 3: Run the targeted test**

Run: `npm --prefix SlideApp test -- --run src/remotion/Composition.test.tsx -t "moves the page 09_img Material and instance explanation into an outside note above the comparison cards"`
Expected: PASS.

### Task 3: Sync notes and page script

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `Docs/剧本/page_09_img-正式动画.md`

**Step 1: Sync storyboard copy**

Update `page_09_img` notes/manuscript so the real sample setup appears in speaker notes.

**Step 2: Sync page script ledger**

Update the page script markdown to mention:
- new top legend text
- moved explanation location
- latest screenshot path and capture provenance

### Task 4: Verify with harness checks

**Files:**
- Verify only

**Step 1: Run storyboard sync audit**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
Expected: emits the page-script audit markdown path and no blocker failure.

**Step 2: Run transition timing audit**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
Expected: emits the timing audit JSON/markdown artifact paths.

**Step 3: Run mechanical review summary**

Run: `npm --prefix SlideApp run review:mechanical`
Expected: summary completes; this task should not add new blocker pages.

**Step 4: Capture the updated page**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --url 'http://127.0.0.1:4175/?step=page_09_img' --prefix page09img-material-instance-note`
Expected: returns a PNG path under `ignore/browser-api-captures/`.
