# Page 19+ Review Harness Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a report-only review command that audits `page_19+` geometry and transition timing using the current harness, writes artifacts under `ignore/`, and never modifies slide content.

**Architecture:** Reuse the existing geometry review artifact builder and the existing Python timing probe instead of inventing a second scoring path. Add a single TypeScript orchestration script under `SlideApp/scripts/` that discovers `page_19+` scope, emits geometry reports for registered sketches, emits timing reports for transitions with workloads, and records missing inputs without failing the command.

**Tech Stack:** TypeScript, Node.js, Vitest, existing slide-geometry review modules, Python timing probe, Markdown, JSON

---

### Task 1: Lock the report-only contract in tests

**Files:**
- Create: `SlideApp/src/review/page19PlusReview.test.ts`
- Read: `SlideApp/src/remotion/pages/lateTailGeometry.test.ts`
- Read: `SlideApp/src/remotion/sceneTimeline.ts`

**Step 1: Write the failing tests**

- Add a test that expects the default reviewed steps to start at `page_19`.
- Add a test that expects missing sketch entries to be reported as `missing_sketch`.
- Add a test that expects missing workload entries to be reported as `missing_workload`.
- Add a test that expects a run to write `summary.json` plus page/transition artifacts into a temp directory.

**Step 2: Run the focused test file and confirm failure**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Expected: FAIL because the review runner does not exist yet.

### Task 2: Extract reusable geometry report shaping helpers

**Files:**
- Create: `SlideApp/src/review/geometryReviewSummary.ts`
- Modify: `SlideApp/scripts/printGeometryReview.ts`
- Test: `SlideApp/src/review/page19PlusReview.test.ts`

**Step 1: Move shared shaping logic out of the CLI script**

- Extract the three-layer review conversion.
- Extract the node/edge issue summarizers.
- Export a helper that returns the JSON-ready geometry summary object.

**Step 2: Keep the existing geometry print script working**

- Update `printGeometryReview.ts` to call the shared helper.

**Step 3: Run focused tests**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Expected: still FAIL, but now only because the page-19+ runner is missing.

### Task 3: Implement page_19+ review discovery and report writing

**Files:**
- Create: `SlideApp/src/review/page19PlusReview.ts`
- Create: `SlideApp/src/review/reviewArtifactWriter.ts`
- Modify: `SlideApp/src/harness/slide-geometry/registry/sketchRegistry.ts`
- Test: `SlideApp/src/review/page19PlusReview.test.ts`

**Step 1: Build scope discovery**

- Read the canonical step sequence.
- Slice it from `page_19` onward.

**Step 2: Add per-page geometry review collection**

- Resolve a formal sketch when present.
- Build geometry artifact summaries using the shared helper.
- Record `missing_sketch` instead of throwing when the step has no registered sketch.

**Step 3: Add adjacent transition timing discovery**

- Pair each reviewed step with its previous step.
- Map transitions to workload JSON paths under `Docs/剧本/workloads/`.
- Record `missing_workload` when no workload file exists.

**Step 4: Add artifact writing**

- Write `summary.json`.
- Write per-page `json + md`.
- Write per-transition `json + md`.

**Step 5: Run the focused tests**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Expected: PASS for scope discovery, missing-input reporting, and artifact writing.

### Task 4: Add the report CLI and package script

**Files:**
- Create: `SlideApp/scripts/reviewSlides.ts`
- Modify: `SlideApp/package.json`
- Test: `SlideApp/src/review/page19PlusReview.test.ts`

**Step 1: Create the CLI entry**

- Parse `--from` with default `page_19`.
- Use `ignore/slide-review/page_19_plus/` as the default output root.
- Print a compact terminal summary and always exit `0`.

**Step 2: Wire the package script**

- Add `review:slides` to `SlideApp/package.json`.

**Step 3: Run the command locally**

Run: `npm --prefix SlideApp run review:slides -- --from page_19`

Expected: PASS with summary output and artifacts under `ignore/slide-review/page_19_plus/`.

### Task 5: Hook timing probe output into the report

**Files:**
- Modify: `SlideApp/src/review/page19PlusReview.ts`
- Test: `SlideApp/src/review/page19PlusReview.test.ts`
- Read: `scripts/slide-geometry-harness/probe_transition_timeline.py`
- Read: `Docs/剧本/workloads/page_18_img_to_page_19.json`
- Read: `Docs/剧本/workloads/page_19_to_page_21.json`

**Step 1: Call the existing Python timing probe**

- Execute `probe_transition_timeline.py` with `--emit-markdown`.
- Parse the JSON payload when available and retain the markdown block for the `.md` artifact.

**Step 2: Keep missing or failing timing probes advisory**

- If the probe command fails, record `probe_error` in the transition result and continue.

**Step 3: Run focused tests and one real command**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Run: `npm --prefix SlideApp run review:slides -- --from page_19`

Expected: PASS; transitions with workloads have timing artifacts, missing ones remain advisory.

### Task 6: Register the current page_19 formal sketch entry

**Files:**
- Create: `SlideApp/src/harness/slide-geometry/contracts/page19-r1.ts`
- Modify: `SlideApp/src/harness/slide-geometry/registry/sketchRegistry.ts`
- Test: `SlideApp/src/review/page19PlusReview.test.ts`

**Step 1: Encode the current merged page 19 formal sketch**

- Add the nodes and edges needed for harness review of the existing merged page 19.
- Preserve the current page structure without altering runtime rendering.

**Step 2: Register the sketch**

- Make `page_19` discoverable by the review command.

**Step 3: Run the focused tests and the report command**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Run: `npm --prefix SlideApp run review:slides -- --from page_19`

Expected: `page_19` now emits a real geometry report instead of `missing_sketch`.

### Task 7: Verify the whole review path

**Files:**
- Generated updates under `ignore/slide-review/page_19_plus/`

**Step 1: Run the focused tests**

Run: `npm --prefix SlideApp test -- src/review/page19PlusReview.test.ts`

Expected: PASS.

**Step 2: Run the report command**

Run: `npm --prefix SlideApp run review:slides -- --from page_19`

Expected: PASS with generated artifacts.

**Step 3: Spot-check that no slide content was changed**

Run: `git status --short`

Expected: only the new review tooling, sketch contract, plan docs, and generated `ignore/` artifacts appear.
