# Page03 Page04 Page04Data QA Band Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a continuous bottom QA band across `page_03`, `page_04`, and `page_04_data`, with question cards on pages 3 and 4 and an answer card on `page_04_data`.

**Architecture:** Reuse existing slide primitives instead of introducing a new visual system. Add a small shared card component for the bottom narrative band, keep the page 3/4 main geometry unchanged, and compress the `page_04_data` table upward so the answer card fits within the same band position.

**Tech Stack:** React 19, Remotion SVG scene rendering, Vitest + Testing Library, markdown storyboard docs.

---

### Task 1: Lock the new QA-band behavior with failing tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write failing tests**

- Add one test for `page_03` and `page_04` question cards.
- Add one test for the `page_04_data` answer card.

**Step 2: Run tests to verify they fail**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "shows the carry-over question cards on page 03 and page 04|answers the precompile and binary questions on page 04 data"`

Expected: FAIL because the new cards and badge test ids do not exist yet.

### Task 2: Implement a shared bottom QA card

**Files:**
- Create: `SlideApp/src/remotion/pages/sharedBottomQaCard.tsx`

**Step 1: Build the shared primitive**

- Wrap `StageBox` plus a top-right `CalloutBadge`.
- Support:
  - card title
  - left-aligned content lines
  - `?` / `!` badge label
  - test ids for card and badge

### Task 3: Render the question cards on `page_03` and `page_04`

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page03Scene.tsx`
- Modify: `SlideApp/src/remotion/pages/Page04Scene.tsx`

**Step 1: Add the cards**

- Render the shared question card at the unified bottom band position.
- Keep the main node and edge layout unchanged.

**Step 2: Use the agreed copy**

- 为什么需要预编译着色器？
- 为什么不能分发 binary？

### Task 4: Render the answer card on `page_04_data`

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page04DataScene.tsx`

**Step 1: Make room for the bottom band**

- Move the table upward and reduce its height to leave room for the answer card.
- Keep all table text at `>=20px`.

**Step 2: Add the shared answer card**

- Render the bottom answer card with `!` badge.
- Use the agreed copy:
  - GPU 编译依赖本地环境：OS / 驱动 / 芯片组合不同。
  - binary 可能都不一样，而且编译很慢。
  - Metal？后面再看 binary archive 2。

### Task 5: Make the tests pass

**Files:**
- No new files beyond the ones above

**Step 1: Run the targeted composition tests**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "shows the carry-over question cards on page 03 and page 04|answers the precompile and binary questions on page 04 data"`

Expected: PASS.

### Task 6: Sync docs and storyboard

**Files:**
- Modify: `Docs/剧本/03-第三页.md`
- Modify: `Docs/剧本/04-第四页.md`
- Modify: `Docs/剧本/05-第五页.md`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Update the script pages**

- Record the new bottom question/answer band.
- Record the new node inventory and geometry constraints on the data page.

**Step 2: Update storyboard manuscript**

- Mention the two questions on pages 3 and 4.
- Mention the data-page answer and the `Metal？` hook.

### Task 7: Run the required closure checks

**Files:**
- No additional source changes expected

**Step 1: Run focused composition tests**

Run: `npm --silent --prefix SlideApp test -- src/remotion/Composition.test.tsx -t "renders page 03 with a top configuration band and API-call legend|renders page 04 as a Vulkan PSO page that keeps the SPIR-V path and middle packaging layers|inserts a dedicated data checkpoint page between page 04 and page 05|shows the carry-over question cards on page 03 and page 04|answers the precompile and binary questions on page 04 data"`

Expected: PASS.

**Step 2: Run storyboard tests**

Run: `npm --silent --prefix SlideApp test -- src/storyboard/pso-storyboard.test.ts`

Expected: PASS.

**Step 3: Run formal-page mechanical review**

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_19`

Expected: no blocker pages introduced.

**Step 4: Run harness audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Expected: audit artifacts generated successfully.

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Expected: storyboard sync audit completes successfully.
