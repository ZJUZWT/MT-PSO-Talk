# Late Tail Content Merge Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Merge the approved late-tail content into `page_19`, `page_24`, and `page_29` without deleting the old late-tail steps yet.

**Architecture:** Lock the new content contract in docs/tests first, then update the late-tail Remotion renderer so the three host pages carry the merged story. Keep the old steps alive in the timeline, but stop treating them as the primary content containers for this material.

**Tech Stack:** TypeScript, React, Remotion, SVG layout primitives, Vitest, slide-geometry-harness audits, Markdown

---

### Task 1: Lock the merge contract in docs and tests

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `Docs/剧本/19-第十九页-正式动画.md`
- Modify: `Docs/剧本/21-第二十一页-正式动画.md`
- Modify: `Docs/剧本/24-第二十四页-正式动画.md`
- Modify: `Docs/剧本/25-第二十五页-正式动画.md`
- Modify: `Docs/剧本/27-第二十七页-正式动画.md`
- Modify: `Docs/剧本/29-第二十九页-正式动画.md`
- Modify: `Docs/剧本/30-第三十页-正式动画.md`

**Step 1: Rewrite storyboard copy**

- Make `page_19` own the binary-instability summary.
- Make `page_24` own the three-row merged strategy summary.
- Make `page_29` own the code-plus-image governance summary.

**Step 2: Update page ledgers**

- Record that `page_21 / page_25 / page_27 / page_30` are temporarily retained for motion continuity, not deleted.

**Step 3: Run storyboard test to confirm the new copy is expected**

Run: `npm --silent --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`

Expected: PASS after the metadata aligns.

### Task 2: Inspect the late-tail renderer and add failing composition assertions

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Modify: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`
- Inspect/modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Add/adjust tests for merged hosts**

- Assert `page_19` now includes the invalidation/binary-instability summary.
- Assert `page_24` now renders three horizontal rows for compression / storage / execution.
- Assert `page_29` renders code blocks with their corresponding images underneath and no tab chrome.

**Step 2: Run focused tests and confirm failure**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 19|page 24|page 29"`

Expected: FAIL before renderer updates.

### Task 3: Implement `page_19` merged content

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify if needed: `SlideApp/src/remotion/pages/page-layout-constants.ts`

**Step 1: Extend `page_19` layout**

- Keep the existing precompile/cache tree.
- Add a compact summary strip or lower-band block that states binary instability and lists the three instability sources.

**Step 2: Preserve `page_21` step**

- Do not remove `page_21` from timeline or anchors.
- Ensure `page_19 -> page_21` still resolves cleanly even after `page_19` carries more content.

**Step 3: Re-run the focused page 19 test**

### Task 4: Implement `page_24` merged three-row strategy layout

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Replace the current left-table/right-cards structure**

- Build three horizontal bands inside `page_24`.
- Row 1: compression label + compact benchmark strip with `LZ4 / ZSTD / Leviathan`.
- Row 2: `LRU + mmap` + optional storage carriers such as `SQL`.
- Row 3: `并行` + `任务独立 / 纯 CPU 计算`.

**Step 2: Keep `page_25 / page_27` alive**

- Do not delete their steps.
- Allow them to remain as secondary pages for now, but not the main hosts for this merged content.

**Step 3: Re-run the focused page 24 test**

### Task 5: Implement `page_29` merged code-plus-image governance layout

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify asset references if needed under: `SlideApp/public/supplement/`

**Step 1: Remove tab framing**

- Render the two code blocks directly in the upper half.

**Step 2: Place the images under their corresponding code blocks**

- Left image under the left code block.
- Right image under the right code block.

**Step 3: Keep `page_30` alive**

- Do not delete the step.
- Let `page_29` become the main merged evidence/governance page.

**Step 4: Re-run the focused page 29 test**

### Task 6: Full verification and harness audits

**Files:**
- Generated outputs under `generated/transition-timing-audit/`
- Updated ledgers under `Docs/剧本/`

**Step 1: Run focused Remotion tests**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx`

**Step 2: Run storyboard sync audit**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

**Step 3: Run transition timing audit**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

**Step 4: Run mechanical summary**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

Expected: no new blockers introduced by the merged-host changes; any remaining blockers should be pre-existing and called out explicitly.
