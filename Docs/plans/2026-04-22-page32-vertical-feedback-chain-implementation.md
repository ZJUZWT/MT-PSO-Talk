# Page 32 Vertical Feedback Chain Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Rebuild `page_32` into a vertical card-and-arrow feedback-chain diagram that reuses the late-tail visual language while preserving the existing tail timing and review pipeline.

**Architecture:** Replace the current text-only bridge body with a centered four-card diagram, then sync the same contract across storyboard metadata, formal review surfaces, tests, and the page ledger so Remotion, harness review, and docs all describe the same structure.

**Tech Stack:** TypeScript, React, Remotion, Vitest, slide-geometry harness audits, Markdown

---

### Task 1: Lock the new `page_32` diagram contract in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`

**Step 1: Update the expected visible contract**

- Expect the page to render:
  - `反馈系统与人的学习`
  - `harness`
  - `loss + back propagation`
  - `feedback system`
  - `Input / f(x) / Output`
- Keep the footer expectation for the soft closing sentence.

**Step 2: Run the focused tests**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "renders page 32 as the feedback bridge page"`

Run: `npm test -- --run src/storyboard/pso-storyboard.test.ts`

Expected: `Composition.test.tsx` passes after implementation; `pso-storyboard.test.ts` may still expose unrelated legacy failures outside `page_32`.

### Task 2: Rebuild the Remotion page as a vertical diagram

**Files:**
- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`

**Step 1: Replace the text-line body model**

- Remove the current 3-line text bridge body layout.
- Define 4 centered diagram cards for:
  - `harness`
  - `loss + back propagation`
  - `feedback system`
  - `Input / f(x) / Output`

**Step 2: Reuse the established late-tail card language**

- Use the same box/fill/stroke vocabulary that already works on `page_31`.
- Keep `harness` and `feedback system` visually emphasized.

**Step 3: Add arrow structure**

- Add 3 downward straight arrows for the main chain.
- Add 1 upward arrow from `feedback system` back to `harness`.
- Keep the return arrow visually secondary so the main reading order stays top-down.

**Step 4: Preserve title and footer behavior**

- Keep the existing title and footer reveal timing windows unless geometry forces a tiny adjustment.
- Do not compress the base page transition timing.

### Task 3: Sync storyboard and formal review surfaces

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`

**Step 1: Rewrite storyboard copy around the diagram**

- Describe the page as a vertical abstract chain instead of a text bridge.
- Keep the same narrative meaning:
  - concrete harness
  - abstraction through `loss + back propagation`
  - `feedback system`
  - push back to `Input / f(x) / Output`
  - soft footer about “看似无用”

**Step 2: Replace the formal sketch contract**

- Remove the old `bridge-line-*` text nodes.
- Add 4 diagram nodes plus the footer and directional edges.
- Make the formal sketch match the real visual arrangement closely enough for geometry review.

### Task 4: Update the page ledger

**Files:**
- Modify: `Docs/剧本/32-第三十二页-正式动画.md`

**Step 1: Rewrite the node inventory**

- Replace the old text-line inventory with:
  - title
  - four diagram cards
  - four directional reading edges / loop edge
  - footer

**Step 2: Rewrite the layout and acceptance sections**

- Document the centered single-column diagram.
- Document the top-down reading order and the secondary loopback arrow.

**Step 3: Keep transition timing sections aligned**

- Preserve or refresh the timing report so the ledger still matches the current `page_31 -> page_32` transition.

### Task 5: Capture and verify

**Files:**
- Refresh generated screenshot assets under `ignore/browser-api-captures/20260422-page32-feedback-bridge/`

**Step 1: Run focused verification**

Run: `npm test -- --run src/remotion/Composition.test.tsx -t "renders page 32 as the feedback bridge page"`

Run: `npm test -- --run src/review/formalPageReviewRegistry.test.ts src/remotion/pages/lateTailGeometry.test.ts`

**Step 2: Capture the updated page**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url 'http://127.0.0.1:4173/?variant=bus-clean&step=page_32' --outdir 'ignore/browser-api-captures/20260422-page32-feedback-bridge' --prefix 'page32-feedback-bridge'`

**Step 3: Run required harness audits**

Run: `python3 scripts/slide-geometry-harness/audit_transition_timings.py`

Run: `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

Run: `npm --silent --prefix SlideApp run review:mechanical -- --from page_10`

Expected: audits pass with `page_32` aligned; `page_31 -> page_32` and `page_32 -> page_33` remain `in_range`; no blocker pages are introduced by the new diagram page.
