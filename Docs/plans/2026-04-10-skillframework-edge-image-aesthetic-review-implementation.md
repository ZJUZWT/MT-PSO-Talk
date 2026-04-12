# SkillFramework Edge Image Aesthetic Review Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Establish a layered skill framework for Edge image capture and image aesthetic analysis, with `Docs/SkillFramework` as the source of truth and mirrored runtime skills under `~/.codex/skills`.

**Architecture:** Create a lower-level contract skill that defines the dual-artifact workflow and ships a sync script. Add one lower-level skill for pulling a local image from the current Microsoft Edge window, one lower-level skill for analyzing a single image's aesthetics, and one higher-level orchestration skill that composes both capabilities into an end-to-end review flow.

**Tech Stack:** Markdown, Python 3, shell, AppleScript, macOS `screencapture`.

---

### Task 1: Create the dual-artifact contract skill

**Files:**
- Create: `Docs/SkillFramework/lower-level/dual-artifact-skill-contract/SKILL.md`
- Create: `Docs/SkillFramework/lower-level/dual-artifact-skill-contract/references/sync-rules.md`
- Create: `Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py`

**Step 1: Write the source-of-truth contract**

Document that project skills must be created in `Docs/SkillFramework/{lower-level,higher-level}` first.

**Step 2: Add a deterministic sync script**

Implement a Python script that mirrors each source skill directory into `~/.codex/skills/<skill-name>`.

**Step 3: Verify the script shape**

Run: `python3 Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py --help`

### Task 2: Create the Edge image fetch skill

**Files:**
- Create: `Docs/SkillFramework/lower-level/fetch-edge-image/SKILL.md`
- Create: `Docs/SkillFramework/lower-level/fetch-edge-image/references/capture-ladder.md`
- Create: `Docs/SkillFramework/lower-level/fetch-edge-image/scripts/capture_edge_window.sh`

**Step 1: Document the capture ladder**

Describe how to prefer direct asset capture when possible and fall back to front-window screenshot capture.

**Step 2: Implement the helper**

Use AppleScript to read the front Edge window title, URL, and bounds, then use `screencapture` to save a local PNG plus metadata.

**Step 3: Verify the helper**

Run: `bash Docs/SkillFramework/lower-level/fetch-edge-image/scripts/capture_edge_window.sh`

### Task 3: Create the image aesthetic analysis skill

**Files:**
- Create: `Docs/SkillFramework/lower-level/analyze-image-aesthetics/SKILL.md`
- Create: `Docs/SkillFramework/lower-level/analyze-image-aesthetics/references/aesthetic-rubric.md`

**Step 1: Define the rubric**

Lock the scoring dimensions, blocker scan, and output format for single-image analysis.

**Step 2: Write the skill**

Keep the workflow scoped to analyzing one actual image rather than code or memory.

### Task 4: Create the orchestration skill

**Files:**
- Create: `Docs/SkillFramework/higher-level/edge-image-aesthetic-review/SKILL.md`
- Create: `Docs/SkillFramework/higher-level/edge-image-aesthetic-review/references/output-template.md`

**Step 1: Compose the lower-level skills**

Document the ordered flow: resolve Edge target, capture a local image, analyze the image, report blockers and next fixes.

**Step 2: Keep the output structured**

Require capture provenance, a verdict, scored dimensions, and prioritized fixes.

### Task 5: Sync and verify runtime mirrors

**Files:**
- Verify: `~/.codex/skills/dual-artifact-skill-contract/SKILL.md`
- Verify: `~/.codex/skills/fetch-edge-image/SKILL.md`
- Verify: `~/.codex/skills/analyze-image-aesthetics/SKILL.md`
- Verify: `~/.codex/skills/edge-image-aesthetic-review/SKILL.md`

**Step 1: Run the sync script**

Run: `python3 Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py`

**Step 2: Compare source and mirror**

Run: `diff -rq Docs/SkillFramework/lower-level/fetch-edge-image ~/.codex/skills/fetch-edge-image`

Repeat for each mirrored skill.
