# Skill-Memory Automation Ledger

This folder is the durable ledger for skill-memory automation: what we observed, what we think is a reusable skill, and what we promoted into `Docs/SkillFramework/`.

There are two ledger behaviors:

- `memory-log.jsonl` and `skill-candidates.jsonl` are canonical JSONL tables updated through deterministic upsert.
- `promotion-log.jsonl` is append-only event history.

## Files

- `memory-log.jsonl`
  - Canonical memory rows worth retaining (facts, stable preferences, constraints, conventions, traps).
- `skill-candidates.jsonl`
  - Canonical proposed skill candidates derived from conversations or repeated work patterns.
- `promotion-log.jsonl`
  - Promotion events when a candidate becomes a real skill under `Docs/SkillFramework/`.

## Schema Essentials

These are intentionally minimal so humans can write them and automation can parse them.

### Shared Reflection Record Fields

These fields are used by the helper scripts in `scripts/skill-memory-automation/`.

Required fields:

- `id` (string): stable row id, e.g. `mem_20260410_page09_lane` or `skc_20260410_hash_lane`
- `timestamp` (string): ISO 8601 timestamp
- `domain` (string): where it applies (`slides`, `skills`, `tooling`, `workflow`, etc.)
- `summary` (string): single durable statement or candidate title
- `source` (string): stable provenance reference (`thread:...`, `Docs/...`, `PR:...`)
- `fingerprint` (string): stable dedupe key
- `status` (string): e.g. `memory`, `candidate`, `promoted`, `rejected`

Optional:

- `evidence` (string or string[]): supporting facts, paths, or snippets
- `occurrence_count` (number): maintained by upsert
- `last_seen_at` (string): maintained by upsert
- `tags` (string[]): freeform labels
- `confidence` (number): `0..1`

### `memory-log.jsonl` record

Memory rows use the shared fields above and may additionally include:

- `memory_type` (string): `fact`, `preference`, `constraint`, `convention`, `trap`

### `skill-candidates.jsonl` record

Required fields:

- all shared reflection record fields
- `level` (string): `lower-level` or `higher-level`.
- `use_when` (string): must start with `Use when...`.
- `problem` (string): what keeps happening.
- `workflow` (string[]): numbered-like steps as array items.
- `outputs` (string[]): required output bullets.
- `hard_bans` (string[]): explicit "do not" constraints.
- `refs` (string[]): links/paths to supporting evidence.
Optional:

- `owner` (string): human/agent responsible for follow-up
- `promoted_at` (string): set by promotion tool
- `promoted_skill_path` (string): set by promotion tool

### `promotion-log.jsonl` record

Required fields:

- `id` (string): generated event id
- `timestamp` (string): ISO 8601 timestamp
- `status` (string): `promoted` or `rejected`
- `domain` (string)
- `summary` (string)
- `source` (string)
- `fingerprint` (string)
- `source_candidate_id` (string)
- `source_candidate_timestamp` (string)

When `result=promoted`, also include:

- `promoted_skill_path` (string): path under `Docs/SkillFramework/.../SKILL.md`

## Promotion Thresholds

Promote a candidate into a real skill under `Docs/SkillFramework/` when all are true:

1. The pattern repeated **at least 2 times** (`occurrence_count >= 2`) or has a clearly scheduled recurrence.
2. The workflow can be written as a stable **5-12 step** procedure.
3. There is at least **one hard ban** that prevents common failure.
4. The skill has explicit **required outputs** so future runs are auditable.
5. References exist (`refs` non-empty) so the skill is grounded in project reality.

Reject or keep as memory-only when:

- it is a one-off request, pure preference, or depends on a single transient context
- it cannot be executed without guessing hidden intent
