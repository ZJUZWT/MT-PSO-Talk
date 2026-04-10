# Capture Rules (Project Memory)

Project memory entries must be:

- **Durable**: likely true for weeks or months, not minutes.
- **Specific**: one sentence you could validate.
- **Actionable**: changes how future work is done.
- **Non-procedural**: not a multi-step workflow (that would be a skill candidate).

## What Counts As Memory

Good examples:

- Write set boundaries: "Worker 1 may only write under `Docs/SkillFramework/**`."
- Tool bans: "Do not edit runtime mirrors under `~/.codex/skills`; sync happens later."
- Repo conventions: "Skill frontmatter descriptions must start with `Use when...`."
- Stable workflow constraints: "Reflection ledgers use deterministic upsert, while promotion-log stays append-only."
- Durable user preference: "Formal semantic lanes must be preserved in slide sketches instead of collapsed into placeholders."

Not memory:

- "This looks nicer right now" (transient taste with no stable rule)
- "We should probably..." (unconfirmed intent)
- One-off status updates ("CI was red today")
- Full procedures with many steps (record a skill candidate instead)

## Classification Fields

Use these fields consistently:

- `area`: one of `docs`, `automation`, `skills`, `slides`, `tooling`, `repo`, `process`, `other`
- `tags`: short labels like `write-set`, `append-only`, `frontmatter`, `provenance`
- `confidence`:
  - `1.0`: explicit instruction or written policy
  - `0.7`: strong convention observed multiple times
  - `0.4`: plausible but not yet confirmed (avoid unless necessary)

## Quality Gate

Before appending:

1. Can the memory be rephrased as a single sentence? If not, split it.
2. Does it reference the concrete thing (path, command, rule) that makes it testable?
3. Is the `source.ref` stable enough for future readers to find it?
