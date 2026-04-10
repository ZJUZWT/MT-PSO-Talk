---
name: capture-project-memory
description: Use when you need to capture a durable, non-skill observation such as a fact, stable preference, constraint, or convention into the project memory ledger.
---

# Capture Project Memory

This skill records small, durable project truths that future work should not have to rediscover.

It does not create new skills; it only captures memory.

## When to Use

Use this skill when:

- a conversation reveals a stable constraint (write sets, tool bans, style rules)
- you learn a repo convention that will matter again (paths, naming, workflow)
- the user states a durable preference that should keep shaping future work
- you spot a repeated pitfall that can be stated as a simple rule

Do not use this skill when the output should be a new skill; use `record-skill-candidate` instead.

## Required Workflow

1. Identify the memory as a single sentence that is stable and testable.
2. Verify it is not already written in an existing skill or ledger entry.
3. Classify it using the rules in [references/rules.md](references/rules.md).
4. Upsert one canonical JSON row into `Docs/SkillFramework/automation/memory-log.jsonl`, preferably through `scripts/skill-memory-automation/upsert_reflection_record.py`.
5. Report the final stored JSON object back to the user.

## Required Output

Always include:

- the appended JSON object (verbatim)
- `domain` and `tags` used
- the `source` value and `fingerprint`

## Hard Bans

- Do not record unstable opinions or one-off situational details as memory.
- Do not store a preference in skill form when it should just be a memory row.
- Do not blur memory with a skill candidate; if it has steps and outputs, it belongs in `skill-candidates.jsonl`.
- Do not omit `source`; provenance is required.

## Reference

See [references/rules.md](references/rules.md) for classification and quality gates.
