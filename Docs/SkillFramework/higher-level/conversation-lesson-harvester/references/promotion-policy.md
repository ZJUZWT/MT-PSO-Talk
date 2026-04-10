# Promotion Policy

Promotion is the act of turning a `skill-candidate` into a real skill under `Docs/SkillFramework/`.

This policy exists to keep the framework small, stable, and auditable.

## Promotion Threshold

Promote when all are true:

1. **Repetition**: observed at least twice (`occurrence_count >= 2`) or has a clearly scheduled recurrence.
2. **Procedural clarity**: can be described as a stable **5-12 step** workflow.
3. **Auditability**: has explicit `outputs` that are checkable.
4. **Failure containment**: has at least one `hard_bans` item that prevents a known common failure.
5. **Grounding**: `refs` is non-empty and points to stable evidence.

## Defer Instead Of Promote When

- The workflow depends on missing tooling or undefined inputs.
- The candidate is too broad ("do X better") and needs narrowing.
- The hard bans are unclear or purely "be careful" statements.

## Reject Instead Of Promote When

- It is primarily a preference.
- It is a one-off situation unlikely to recur.
- The workflow requires guessing intent or private context.

## Logging

When you decide to promote, append one line to `Docs/SkillFramework/automation/promotion-log.jsonl` with:

- `timestamp`, `source_candidate_id`, `status=promoted`, `reason`
- `promoted_skill_path`

When you decide to reject, append one line with:

- `timestamp`, `source_candidate_id`, `status=rejected`, `reason`
