# Vulkan Page 04 Description Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Update page 04 so the visual, manuscript, and tests all explain `SPIR-V -> Description -> PSO -> GPU`, with `PSO -> GPU` as the only highlighted runtime API call.

**Architecture:** Keep page 03 geometry as the inherited skeleton, then change only the page-04-specific upper-band labels, inserted middle layer, and manuscript copy. Use TDD for both storyboard text and remotion rendering expectations.

**Tech Stack:** React, SVG-in-Remotion composition, Vitest, Testing Library

---

### Task 1: Lock the new Vulkan page-04 story in tests

**Files:**
- Modify: `SlideApp/src/storyboard/pso-storyboard.test.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Steps:**
1. Add a failing storyboard assertion for `SPIR-V`, `Description`, and the revised Vulkan manuscript.
2. Run the targeted tests and confirm they fail.
3. Add a failing remotion assertion that page 04 renders `SPIR-V` and `Description`, not `ShaderBinary`.
4. Run the targeted remotion tests and confirm they fail.

### Task 2: Implement the new page-04 data model and copy

**Files:**
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `Docs/剧本/04-第四页.md`
- Modify: `Docs/剧本/README.md`

**Steps:**
1. Update the page-04 storyboard copy to describe `SPIR-V`, `Description`, create, and bind semantics.
2. Update the Chinese script doc to match the new design rationale.
3. Keep the page-03 copy unchanged so the contrast remains explicit.

### Task 3: Implement the remotion diagram changes

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Steps:**
1. Introduce a `Description` inserted layer that can fit between the upper band and PSO without breaking page continuity.
2. Rename the page-04 compiled artifact label to `SPIR-V`.
3. Re-route the upper gray lines so the Vulkan page reads as inputs feeding `Description`, then creating `PSO`.
4. Preserve `PSO -> GPU` as the only orange runtime call.

### Task 4: Verify and close out

**Files:**
- Verify only

**Steps:**
1. Run targeted tests for storyboard and remotion rendering.
2. Run the full `SlideApp` test suite.
3. Run the `SlideApp` build.

## Result

- Implemented the Vulkan page-04 semantic shift to `SPIR-V -> Description -> PSO -> GPU`.
- Refined the page-03 to page-04 spatial change so page 03 feels more compressed while page 04 opens vertically.
- Verified with `npm --prefix /Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp run test` and `npm --prefix /Users/swannzhang/Workspace/AIProjects/MT-PSO-Talk/SlideApp run build`.
