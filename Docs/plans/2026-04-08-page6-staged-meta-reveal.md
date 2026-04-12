# Page 06 Staged Meta Reveal Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make page 06 reveal in three stages: main ownership chain including `Cooked ShaderCode` first, dashed guide lines second, and the white info boards last.

**Architecture:** Keep the settled page 06 layout unchanged and only split the current `page6MetaOpacity` behavior into two additional animation channels. The left ownership spine remains a rigid skeleton, while dashed relations get their own reveal progress and the three white boards plus their text/shadows get a later board reveal progress.

**Tech Stack:** React, Remotion SVG composition, Vitest.

---

### Task 1: Lock the page 06 reveal order in tests

**Files:**
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Write the failing tests**
- Add a test that samples an early frame where the page 06 stage is already visible and asserts the main chain nodes plus `Cooked ShaderCode` are present while dashed guides and white boards are still absent.
- Add a test that samples a mid frame where dashed guides are visible but the white boards are still effectively hidden.

**Step 2: Run test to verify it fails**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: FAIL because dashed guides and boards currently share the same reveal timing.

### Task 2: Split page 06 meta timing

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`

**Step 1: Write minimal implementation**
- Introduce a local page 06 reveal progress for the page 05 -> page 06 transition.
- Derive a dashed reveal progress that starts after the main chain is on screen.
- Derive a board reveal progress that starts after dashed guides finish stretching.
- Use the dashed progress to grow the dashed paths from the main chain outward.
- Use the board progress for the three white boards, their text, and their shadows.

**Step 2: Run focused tests**

Run: `npm test -- src/remotion/Composition.test.tsx`
Expected: PASS.

### Task 3: Full verification

**Files:**
- Modify: `SlideApp/src/remotion/Composition.tsx`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`

**Step 1: Run complete checks**

Run: `npm test`
Expected: PASS.

**Step 2: Run production build**

Run: `npm run build`
Expected: PASS.
