# SlideApp Browser Capture Bridge Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Replace the fragile screen-region screenshot path with a browser-native capture bridge that can export either the full SlideApp page or the stage runtime to a stable local PNG file.

**Architecture:** Reuse the existing `html-to-image` capture logic inside `SlideApp`, but expose it through a query-driven export flow that can run without a manual button click. Add a small local receiver script that accepts the exported PNG from the page, writes it to disk with metadata, and prints the resulting file path so harness scripts can consume it mechanically.

**Tech Stack:** React, TypeScript, Vitest, `html-to-image`, Python 3, shell, Microsoft Edge headless.

---

### Task 1: Define the browser-side capture export contract

**Files:**
- Modify: `SlideApp/src/App.tsx`
- Modify: `SlideApp/src/utils/captureImage.ts`
- Test: `SlideApp/src/App.test.tsx`
- Test: `SlideApp/src/utils/captureImage.test.ts`

**Step 1: Write the failing tests**

- Add an app test that loads a query such as `?capture=1&captureScope=page&captureTransport=post&capturePostUrl=http://127.0.0.1:9999/capture` and expects the app to export `.workbench-shell` automatically without pressing `!`.
- Add a second app test for `captureScope=stage` in review or sketch mode and expect the export target to be `.stage-runtime`.
- Add a utility test that verifies a PNG blob can be posted to an HTTP endpoint with metadata.

**Step 2: Run tests to verify they fail**

Run: `npm run slide:test -- SlideApp/src/App.test.tsx SlideApp/src/utils/captureImage.test.ts`

Expected: FAIL because no query-driven export behavior or POST helper exists yet.

**Step 3: Write the minimal implementation**

- Extract a reusable `captureElementToBlob()` helper from the current button flow.
- Add a small export helper that can either:
  - write the PNG to clipboard/download for UI buttons
  - or POST the PNG plus metadata for automation
- Parse new query params in `App.tsx`:
  - `capture=1`
  - `captureScope=page|stage`
  - `captureTransport=post`
  - `capturePostUrl=<url>`
- Trigger the export once the target element is mounted and stable.

**Step 4: Run tests to verify they pass**

Run: `npm run slide:test -- SlideApp/src/App.test.tsx SlideApp/src/utils/captureImage.test.ts`

Expected: PASS.

### Task 2: Add a local PNG receiver for automation

**Files:**
- Create: `scripts/slide-geometry-harness/receive_capture_png.py`
- Modify: `scripts/slide-geometry-harness/capture_review_png.sh`
- Test: manual command verification

**Step 1: Write the failing verification scenario**

- Define the desired command shape:
  - `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "<slide-url>"`
- Verify the command currently fails because no browser API receiver mode exists.

**Step 2: Run the failing command**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "http://127.0.0.1:4173/?step=page_09"`

Expected: FAIL with unsupported mode or missing receiver.

**Step 3: Write the minimal implementation**

- Add a tiny Python HTTP server that:
  - listens on localhost
  - accepts one POST
  - writes the PNG and metadata to `ignore/browser-api-captures/<timestamp>/`
  - prints the final PNG path
- Extend the shell wrapper to:
  - start the receiver
  - build a capture URL with the query params from Task 1
  - launch Edge headless against that URL
  - wait for the receiver result

**Step 4: Run the command to verify it passes**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "http://127.0.0.1:4173/?step=page_09&mode=sketch&sketch=page09-r1&review=1"`

Expected: PASS and print a local PNG path.

### Task 3: Preserve existing button behavior while adding automation support

**Files:**
- Modify: `SlideApp/src/components/CaptureClipboardButton.tsx`
- Modify: `SlideApp/src/utils/captureImage.ts`
- Test: `SlideApp/src/App.test.tsx`

**Step 1: Write the failing regression expectation**

- Verify the floating `!` button still captures the full page.
- Verify the review HUD button still captures only the stage.

**Step 2: Run tests to verify they fail only if the refactor broke them**

Run: `npm run slide:test -- SlideApp/src/App.test.tsx`

Expected: Existing capture tests continue to pass or surface any regression immediately.

**Step 3: Refactor minimally**

- Reuse the shared blob capture helper so button capture and automation export share the same rasterization path.
- Keep the current UX copy and fallback-to-download behavior unchanged.

**Step 4: Run tests to verify they pass**

Run: `npm run slide:test -- SlideApp/src/App.test.tsx`

Expected: PASS.

### Task 4: Verify the new page capture path end-to-end

**Files:**
- Verify only

**Step 1: Run targeted tests**

Run: `npm run slide:test -- SlideApp/src/App.test.tsx SlideApp/src/utils/captureImage.test.ts`

Expected: PASS.

**Step 2: Run the full-page capture command**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "http://127.0.0.1:4173/?step=page_09&mode=sketch&sketch=page09-r1&review=1"`

Expected: PASS and save a whole-page PNG plus metadata.

**Step 3: Run the stage capture command**

Run: `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope stage --url "http://127.0.0.1:4173/?step=page_09&mode=sketch&sketch=page09-r1&review=1"`

Expected: PASS and save a stage-only PNG plus metadata.

**Step 4: Confirm fallback status**

- Keep `front-window` as a legacy/manual path for now.
- Prefer `browser-api` for agent automation once verification succeeds.
