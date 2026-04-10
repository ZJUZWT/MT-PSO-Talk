# Skill Candidate Schema

Each line in `Docs/SkillFramework/automation/skill-candidates.jsonl` is one JSON object. The helper script rewrites the ledger as a canonical table, so future rows replace older state for the same fingerprint instead of preserving every revision.

## Required Fields

- `id` (string): stable id such as `skc_YYYYMMDD_HHMM_<slug>`.
- `timestamp` (string): ISO 8601 timestamp.
- `domain` (string): e.g. `slides`, `workflow`, `skills`.
- `summary` (string): short name or one-line summary.
- `source` (string): stable provenance pointer.
- `fingerprint` (string): dedupe key.
- `status` (string): typically `candidate` before promotion.
- `level` (string): `lower-level` or `higher-level`.
- `use_when` (string): must start with `Use when...`.
- `problem` (string): short paragraph describing the repeated pain.
- `workflow` (string[]): 5-12 items; each is one imperative step.
- `outputs` (string[]): required outputs; short phrases.
- `hard_bans` (string[]): explicit "Do not..." rules.
- `refs` (string[]): at least 1 entry; paths, URLs, PR ids, message ids.
- `refs` (string[]): at least 1 entry; paths, URLs, PR ids, message ids.

## Optional Fields

- `owner` (string): who will follow up
- `evidence` (string or string[]): support that helps future promotion
- `occurrence_count` (number): maintained by upsert
- `last_seen_at` (string): maintained by upsert
- `promoted_at` (string): set during promotion
- `promoted_skill_path` (string): set during promotion

## Constraints

- Keep `use_when` one sentence and start it with `Use when...`.
- Prefer `lower-level` when the skill is a single operation with one main output.
- Prefer `higher-level` when the skill orchestrates other skills or has multiple phases.
- `hard_bans` is mandatory; if you cannot name at least one, the candidate is not ready.

## Example (Shape Only)

```json
{
  "id": "skc_20260410_2000_example",
  "timestamp": "2026-04-10T20:00:00+08:00",
  "domain": "workflow",
  "summary": "Example Skill",
  "source": "thread:example",
  "fingerprint": "workflow::example-skill",
  "status": "candidate",
  "level": "lower-level",
  "use_when": "Use when you need to ...",
  "problem": "What keeps happening and why it hurts.",
  "workflow": ["Step 1", "Step 2"],
  "outputs": ["Output A"],
  "hard_bans": ["Do not ..."],
  "refs": ["Docs/.../something.md"],
  "occurrence_count": 2,
  "last_seen_at": "2026-04-10T20:00:00+08:00"
}
```
