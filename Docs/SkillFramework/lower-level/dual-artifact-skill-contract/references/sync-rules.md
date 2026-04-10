# Sync Rules

## Directory Contract

Every project skill must start in exactly one of these source locations:

- `Docs/SkillFramework/lower-level/<skill-name>/`
- `Docs/SkillFramework/higher-level/<skill-name>/`

Each source skill directory must contain:

- `SKILL.md`

Each source skill directory may also contain:

- `references/`
- `scripts/`

Each source skill is mirrored into:

- `~/.codex/skills/<skill-name>/`

The layer name is intentionally not preserved in the runtime mirror path. Runtime lookup is by skill name only.

## Mirror Expectations

- Folder name matches frontmatter `name`
- `SKILL.md` exists in source and runtime
- helper scripts and references are copied with the skill
- runtime mirror is treated as generated output for this repo workflow

## Verification Commands

Sync all repo skills:

```bash
python3 Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py
```

Verify one skill:

```bash
diff -rq Docs/SkillFramework/lower-level/fetch-edge-image ~/.codex/skills/fetch-edge-image
```

Verify a higher-level skill:

```bash
diff -rq Docs/SkillFramework/higher-level/edge-image-aesthetic-review ~/.codex/skills/edge-image-aesthetic-review
```

## Decision Rule

Use `lower-level` for single-capability skills such as capture, parsing, scoring, formatting, or extraction.

Use `higher-level` for skills whose main value is orchestration, ordering, or summarizing multiple lower-level outputs.
