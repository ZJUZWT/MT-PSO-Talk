# Harness Workflow Gate Design

## Goal

Give the slide harness a real flow-layer entrypoint inside the repo so agents do not rely only on `AGENTS.md` prose to decide whether to use it.

## Problem

- `AGENTS.md` communicates policy, but it is still a prompt-layer rule.
- The current `slide-geometry-harness` workflow is rich but expensive, so agents are tempted to skip it for smaller edits.
- The repo already has strong audit scripts, but they are not wrapped in a lightweight task classifier that can be called from an agent hook or completion gate.

## Design

Introduce a single repo-local Python entrypoint:

- `scripts/slide-geometry-harness/workflow_gate.py`

It provides two minimal subcommands:

1. `classify`
   - Inputs: prompt text and/or changed file paths.
   - Output: one of `skip`, `lite`, or `full`, plus reasons and required commands.
   - Optional: persist a small JSON state file so later phases can reuse the decision.

2. `stop`
   - Inputs: a previously persisted state file.
   - Output: run the required audit commands for the chosen harness level.
   - `skip` runs nothing.
   - `lite` runs the doc/storyboard alignment audit.
   - `full` runs both timing audit and doc/storyboard alignment audit.

## Scope

This first version deliberately does not hard-code any Claude Code or Codex hook file format.
It only defines the repo-side logic that a host hook can call.

## Classification rules

### `full`

Use `full` when the prompt or changed files indicate real geometry / animation work, especially:

- `SlideApp/src/remotion/**`
- `SlideApp/src/harness/slide-geometry/**`
- `Docs/剧本/**`
- `Docs/剧本/workloads/**`
- `scripts/slide-geometry-harness/**`
- prompt mentions like `动画`, `过渡`, `几何`, `layout`, `node`, `edge`, `timeline`, `embed.ts`

### `lite`

Use `lite` for slide-adjacent changes that still affect page meaning or presentation but do not obviously require full geometry review, especially:

- `SlideApp/src/storyboard-data/**`
- `SlideApp/src/components/NotesPanel.tsx`
- `SlideApp/src/components/ProgressBubbles.tsx`
- prompt mentions like `左边卡片`, `文案`, `notes`, `copy`, `caption`, `重点`

### `skip`

Use `skip` when neither prompt nor files suggest slide/harness work.

## Required commands by level

### `lite`

- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

### `full`

- `python3 scripts/slide-geometry-harness/audit_transition_timings.py`
- `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`

## Repo integration

Point repo guidance at the new gate:

- update `AGENTS.md` to mention the workflow gate as the preferred flow-layer entrypoint
- update `Docs/剧本/README.md` with a short “flow-layer gate” command example

## Verification

- Python `unittest` for classification and stop-plan generation
- one build-safe pass to confirm repo changes do not disturb `SlideApp`
