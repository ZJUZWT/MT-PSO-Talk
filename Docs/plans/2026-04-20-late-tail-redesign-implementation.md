# SlideApp Late Tail Redesign Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the current placeholder ending chain with the approved late-tail chapter: four strategy pages, three governance pages, one harness page, one reading page, and one final quote page, while deleting `page_23`.

**Architecture:** Update the canonical storyboard/timeline first, then replace the late placeholder renderers inside `Page10Scene` with page-specific layouts that reuse the existing board shell. After that, retime the new late-step anchors, run harness timing probes, and write the updated facts back into `Docs/剧本/`.

**Tech Stack:** TypeScript, React, Remotion, SVG primitives, Vitest, slide-geometry-harness probe scripts, Markdown

---

### Task 1: Lock the new late-step contract in tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing assertions**

- Remove `page_23` from the expected canonical order.
- Extend the canonical order to `page_33`.
- Assert the new late pages contain the approved titles and core copy.

**Step 2: Run focused tests and confirm failure**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts src/remotion/sceneTimeline.test.ts src/remotion/Composition.test.tsx`

Expected: FAIL because the old late tail still exists.

### Task 2: Update storyboard, step ids, and frame anchors

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-workbench-types.ts`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/remotion/embed.ts`
- Modify: `SlideApp/src/remotion/sceneTimeline.ts`
- Modify: `SlideApp/src/remotion/pages/page-layout-constants.ts`

**Step 1: Remove `page_23` and add `page_30` ~ `page_33`**

- Update type unions and step sequences.

**Step 2: Rewrite late storyboard copy**

- Replace placeholder late-page metadata with approved strategy/governance/harness/reading/quote content.

**Step 3: Retime the tail**

- Re-anchor late-step frames so the new chain has enough hold time and does not compress prior motion.

### Task 3: Render the new late pages

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Add reusable late-page helpers**

- Add helpers for page titles, section cards, code blocks, metric pills, and image cards.

**Step 2: Replace placeholder tail renderers**

- Keep the existing board-shell approach.
- Render the 10 late pages with their real structure and supplied assets.

**Step 3: Update late reveal/focus logic**

- Delete `page_23` timing and visibility branches.
- Thread the new `page_30` ~ `page_33` reveals into the suppression chain.

### Task 4: Copy and wire assets

**Files:**
- Create/modify under: `SlideApp/public/supplement/`

**Step 1: Copy governance screenshots to public assets**

- Normalize filenames for the browser-facing asset paths.

**Step 2: Reference those public URLs from the new governance pages**

- Keep image usage local to the new late-page components.

### Task 5: Update late-page scripts and timing facts

**Files:**
- Delete: `Docs/剧本/23-第二十三页-正式动画.md`
- Modify/create: `Docs/剧本/24-第二十四页-正式动画.md`
- Modify/create: `Docs/剧本/25-第二十五页-正式动画.md`
- Create: `Docs/剧本/26-第二十六页-正式动画.md`
- Create: `Docs/剧本/27-第二十七页-正式动画.md`
- Create: `Docs/剧本/28-第二十八页-正式动画.md`
- Create: `Docs/剧本/29-第二十九页-正式动画.md`
- Create: `Docs/剧本/30-第三十页-正式动画.md`
- Create: `Docs/剧本/31-第三十一页-正式动画.md`
- Create: `Docs/剧本/32-第三十二页-正式动画.md`
- Create: `Docs/剧本/33-第三十三页-正式动画.md`
- Create/modify: `Docs/剧本/workloads/*.json`

**Step 1: Create/update page ledgers**

- Write node/edge summaries and change notes for every edited late page.

**Step 2: Run probe-driven timing review**

- Generate workload JSON for each adjacent late transition.
- Run `probe_transition_timeline.py` and paste the markdown output into the page ledgers.

### Task 6: Verify and audit

**Files:**
- Generated updates under `Docs/剧本/`

**Step 1: Run tests**

Run: `npm --prefix SlideApp test`

**Step 2: Run build**

Run: `npm --prefix SlideApp run build`

**Step 3: Run harness audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: the late tail is synchronized and the new transition timings are within range or explicitly flagged for anchor adjustment.
