# Page 30 PSO Reading Split Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Split the current PSO engineering reading block out of the mixed recommendation page and make it a standalone page immediately after `page_29`, while keeping later pages in place.

**Architecture:** Reuse the existing hidden `page_30` slot as the new standalone `PSO 延伸阅读` page so the late-tail step ids stay stable. Move the four PSO-oriented engineering links from the current `page_32` reading page into `page_30`, then simplify `page_32` into the remaining book/video/game recommendation page and update the late-tail docs, formal review surfaces, and focused tests to match.

**Tech Stack:** Remotion React scenes, storyboard metadata, formal review registry, Vitest, slide-geometry harness audits.

---

### Task 1: Reframe the late-tail storyboard metadata

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Update the page metadata**

- Change `page_30` from the hidden governance carry page into the visible standalone `PSO 延伸阅读` page.
- Keep `page_31`, `page_32`, and `page_33` ids unchanged.
- Rewrite `page_32` metadata so it no longer claims to contain the four PSO engineering links.

**Step 2: Update late-tail storyboard expectations**

Run: `npm --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`

Expected: PASS with updated page labels / hidden flags / related-link expectations.

### Task 2: Move the actual reading content in the Remotion page scene

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Reassign the engineering link card**

- Render the four PSO engineering links on `page_30`.
- Remove that engineering card from `page_32`.
- Keep `page_32` as the lighter “book / video / game” page.

**Step 2: Update focused rendering tests**

Run: `npm --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "page 30|page 31|page 32|page 33"`

Expected: PASS with `page_30` checking the four PSO links, and `page_32` checking only the remaining recommendation content.

### Task 3: Sync formal review surfaces and page scripts

**Files:**
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `Docs/剧本/30-第三十页-正式动画.md`
- Modify: `Docs/剧本/31-第三十一页-正式动画.md`
- Modify: `Docs/剧本/32-第三十二页-正式动画.md`
- Modify: `Docs/剧本/33-第三十三页-正式动画.md`

**Step 1: Refresh the formal review surfaces**

- Update `formal-page30` to match the standalone reading page structure.
- Update `formal-page32` to match the reduced recommendation page structure.

**Step 2: Refresh the page scripts**

- `page_30`: describe it as the standalone PSO ending / reading page.
- `page_31`: update previous-page references to `page_30`.
- `page_32`: remove the engineering-card narrative and keep only book / video / game content.
- `page_33`: update previous-page continuity if needed.

### Task 4: Run the required harness checks and close the loop

**Files:**
- Modify if needed: `Docs/剧本/00-剧本与页面对齐审计.md`

**Step 1: Run focused mechanical review**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_29`

Expected: edited late-tail pages remain `status=ok` / `blockerOpen=false`.

**Step 2: Run timing audit**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Expected: audit artifact emitted successfully.

**Step 3: Run storyboard sync audit**

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: audit markdown path printed with no error.

**Step 4: Close workflow gate**

Run: `python3 scripts/slide-geometry-harness/workflow_gate.py stop --state-path .git/slide-geometry-harness-workflow-state.json`

Expected: `missing_focus_steps: []` and no blocker pages for this turn.
