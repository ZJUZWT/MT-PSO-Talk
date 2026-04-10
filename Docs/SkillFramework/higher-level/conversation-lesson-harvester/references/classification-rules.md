# Classification Rules

Classify each extracted lesson as exactly one of:

- `memory`
- `skill-candidate`
- `ignore`

## Choose `memory` When

- It is a stable fact, constraint, convention, or policy.
- It can be written as a single sentence.
- It changes future behavior but does not require a multi-step procedure.

Examples:

- write set boundaries
- "append-only" ledger rules
- naming conventions for skills
- stable user preferences that should shape future work

## Choose `skill-candidate` When

- The lesson is a repeatable workflow with multiple steps.
- It has explicit required outputs (so future runs are auditable).
- It has at least one hard ban to prevent common failure.
- It would benefit from being executed again in future work.

Examples:

- capture -> analyze -> report workflows
- quality gates that require a structured checklist

## Choose `ignore` When

- It is a preference without a stable rule.
- It is a one-off status update.
- It depends on private context not available to future runs.
- It cannot be executed without guessing hidden intent.

## Split Rule

If a lesson contains both:

- a durable constraint (memory), and
- a repeatable procedure (skill),

then split it into two artifacts: one `memory` entry and one `skill-candidate`.
