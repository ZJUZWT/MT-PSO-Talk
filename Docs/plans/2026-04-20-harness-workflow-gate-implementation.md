# Harness Workflow Gate Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add a repo-local workflow gate that classifies slide work into `skip / lite / full` and drives the right harness audits from a future agent hook.

**Architecture:** Keep the first version host-agnostic. The repo will expose a single Python CLI under `scripts/slide-geometry-harness/` that accepts prompt/files, persists a small state file, and can later run the right audits on stop. This gives Codex, Claude Code, or a future wrapper the same flow-layer core without duplicating logic.

**Tech Stack:** Python 3.11, `unittest`, `subprocess`, `json`, existing harness audit scripts.

---

### Task 1: Add failing tests for workflow classification

**Files:**
- Create: `scripts/slide-geometry-harness/tests/test_workflow_gate.py`
- Test: `scripts/slide-geometry-harness/tests/test_workflow_gate.py`

**Step 1: Write the failing tests**

Cover:
- remotion/geometry changes classify as `full`
- notes/storyboard copy changes classify as `lite`
- unrelated repo changes classify as `skip`
- persisted state is readable by the stop planner

**Step 2: Run test to verify it fails**

Run:
```bash
python3 -m unittest discover -s scripts/slide-geometry-harness/tests -p 'test_*.py' -v
```

Expected:
- import or symbol failures because `workflow_gate.py` does not exist yet

### Task 2: Implement the workflow gate CLI

**Files:**
- Create: `scripts/slide-geometry-harness/workflow_gate.py`
- Test: `scripts/slide-geometry-harness/tests/test_workflow_gate.py`

**Step 1: Write minimal implementation**

Implement:
- `classify_workflow(prompt, files) -> decision`
- `build_stop_plan(level, repo_root) -> commands`
- `write_state(...)`
- CLI subcommands:
  - `classify`
  - `stop`

**Step 2: Run tests to verify they pass**

Run:
```bash
python3 -m unittest discover -s scripts/slide-geometry-harness/tests -p 'test_*.py' -v
```

Expected:
- all tests pass

### Task 3: Document the flow-layer entrypoint

**Files:**
- Modify: `AGENTS.md`
- Modify: `Docs/剧本/README.md`

**Step 1: Add minimal guidance**

Document:
- the workflow gate command
- the `skip / lite / full` levels
- the default audits for `lite` and `full`

**Step 2: Verify docs stay aligned with the script behavior**

Run:
```bash
python3 scripts/slide-geometry-harness/workflow_gate.py classify --prompt "修改 page_31 左边卡片文案"
```

Expected:
- emits a `lite` decision with the storyboard sync audit in the command list

### Task 4: Do a build-safe verification pass

**Files:**
- Verify only

**Step 1: Run unit tests**

Run:
```bash
python3 -m unittest discover -s scripts/slide-geometry-harness/tests -p 'test_*.py' -v
```

Expected:
- PASS

**Step 2: Run a SlideApp safety build**

Run:
```bash
npm --prefix SlideApp run build
```

Expected:
- PASS
