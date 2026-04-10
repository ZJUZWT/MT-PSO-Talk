---
name: dual-artifact-skill-contract
description: Use when creating or editing project skills that must exist both under Docs/SkillFramework and in ~/.codex/skills, or when deciding whether a skill belongs to lower-level or higher-level.
---

# Dual Artifact Skill Contract

This skill keeps project skill sources and runtime skill mirrors aligned.

For this project, the source of truth lives under:

- `Docs/SkillFramework/lower-level/<skill-name>/`
- `Docs/SkillFramework/higher-level/<skill-name>/`

The runtime mirror lives under:

- `~/.codex/skills/<skill-name>/`

## When to Use

Use this skill when:

- adding a new project skill
- editing an existing project skill
- deciding whether a skill is `lower-level` or `higher-level`
- syncing repo skill sources into runtime skill directories

Do not use this skill for personal-only skills that intentionally live outside this repo.

## Layer Rule

Choose `lower-level` when the skill owns one stable capability with a clear input and output contract.

Choose `higher-level` when the skill mainly sequences or composes 2 or more lower-level skills.

If a skill can do useful work independently, default it to `lower-level`.

## Core Workflow

1. Create or edit the source skill first under `Docs/SkillFramework`.
2. Keep the folder name equal to the skill `name` in frontmatter.
3. Store only the files the skill actually needs: `SKILL.md`, optional `references/`, optional `scripts/`.
4. Run the sync script:

```bash
python3 Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py
```

5. Verify the mirrored runtime directory exists and matches the source.

## Required Output

When using this skill, report:

- the chosen layer and why
- the source skill path
- the runtime mirror path
- whether sync ran successfully
- whether mirror verification passed

## Hard Bans

- Do not create a runtime-only project skill.
- Do not edit `~/.codex/skills/<skill-name>` first and promise to backport later.
- Do not combine a reusable base capability and an orchestration workflow in one skill when the split is clear.
- Do not leave source and runtime copies drifting after the task ends.

## Reference

See [references/sync-rules.md](references/sync-rules.md) for the directory contract and mirror checks.
