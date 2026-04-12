# Clarifying With Sketches Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Create a global Codex skill that forces clarification, quick sketching, numbered-requirement mapping, and layered critique summaries before visual or structural implementation work.

**Architecture:** The implementation has two layers: first document the workflow in repo-local design artifacts for future reference, then install a single global skill under `$CODEX_HOME/skills` that encodes the workflow as a rigid pre-implementation process. Validation will focus on trigger quality, workflow completeness, and presence of explicit skip conditions.

**Tech Stack:** Markdown skill authoring, local filesystem skill installation, manual validation via file review.

---

### Task 1: Create the approved design record

**Files:**
- Create: `Docs/plans/2026-04-07-clarifying-with-sketches-design.md`

**Step 1: Write the design doc**

Capture:

- why the skill is needed
- when it should trigger
- the mandatory clarify-before-code rule
- the sketch-before-code rule
- the numbered-requirement self-review rule
- the three-layer critique summary rule
- the explicit skip condition rule

**Step 2: Verify the design doc exists**

Run: `test -f Docs/plans/2026-04-07-clarifying-with-sketches-design.md`
Expected: command succeeds.

### Task 2: Create the global skill

**Files:**
- Create: `/Users/swannzhang/.codex/skills/clarifying-with-sketches/SKILL.md`

**Step 1: Write the skill frontmatter**

Add:

- `name: clarifying-with-sketches`
- a trigger description focused on visual-structure and ambiguity-heavy tasks

**Step 2: Write the workflow body**

Include:

- when to use
- when not to use
- required clarification loop
- required quick sketch step before code edits
- `<cwd>/ignore/` storage convention
- numbered-requirement mapping template
- three-layer critique summary template
- explicit skip-condition handling

**Step 3: Keep the skill concise**

Avoid references that are not essential to this workflow. Prefer one self-contained `SKILL.md`.

### Task 3: Validate the skill contract

**Files:**
- Read: `/Users/swannzhang/.codex/skills/clarifying-with-sketches/SKILL.md`

**Step 1: Validate trigger coverage**

Confirm the description clearly covers:

- layout tasks
- diagrams
- animation
- spatial ambiguity
- numbered requirements

**Step 2: Validate hard rules**

Confirm the body explicitly requires:

- asking clarifying questions when unclear
- sketch before code
- post-sketch requirement mapping
- layered critique summary after rejection
- skip behavior only when the user explicitly allows it

**Step 3: Validate storage convention**

Confirm sketch artifacts are directed to `<cwd>/ignore/`.

### Task 4: Record the implementation plan in repo

**Files:**
- Create: `Docs/plans/2026-04-07-clarifying-with-sketches-implementation.md`

**Step 1: Save this plan**

Make sure the repo contains the implementation handoff for later refinement.

**Step 2: Verify the file exists**

Run: `test -f Docs/plans/2026-04-07-clarifying-with-sketches-implementation.md`
Expected: command succeeds.

### Task 5: Final verification

**Files:**
- Read: `Docs/plans/2026-04-07-clarifying-with-sketches-design.md`
- Read: `Docs/plans/2026-04-07-clarifying-with-sketches-implementation.md`
- Read: `/Users/swannzhang/.codex/skills/clarifying-with-sketches/SKILL.md`

**Step 1: Run file existence checks**

Run:

```bash
test -f Docs/plans/2026-04-07-clarifying-with-sketches-design.md
test -f Docs/plans/2026-04-07-clarifying-with-sketches-implementation.md
test -f /Users/swannzhang/.codex/skills/clarifying-with-sketches/SKILL.md
```

Expected: all commands succeed.

**Step 2: Review for workflow integrity**

Confirm the installed skill matches the approved design:

- clarify first
- sketch second
- map numbered requirements
- summarize critique in three layers
- allow explicit skip only when requested

