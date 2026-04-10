---
name: record-skill-candidate
description: Use when you have a repeatable workflow with required outputs and hard bans that should be recorded as a candidate skill for later promotion.
---

# Record Skill Candidate

This skill records a proposed skill as a candidate in the canonical candidate ledger.

It does not promote the skill; promotion is a separate step tracked in `promotion-log.jsonl`.

## When to Use

Use this skill when:

- the same task pattern is showing up repeatedly
- the workflow can be written as stable steps (not improvisation)
- there are clear failure modes that should become hard bans
- you want an auditable candidate before turning it into a formal `SKILL.md`

Do not use this skill to store one-off notes; use `capture-project-memory` for that.

## Required Workflow

1. Draft the skill's `use_when` sentence (must start with `Use when...`).
2. Decide the skill `level`: `lower-level` (single operation) vs `higher-level` (orchestrates other skills).
3. Write:
   - `problem` in one paragraph
   - `workflow` as an array of 5-12 steps
   - `outputs` as an array of required outputs
   - `hard_bans` as an array of explicit "do not" rules
4. Fill provenance references in `refs`.
5. Upsert one canonical JSON row into `Docs/SkillFramework/automation/skill-candidates.jsonl`, preferably through `scripts/skill-memory-automation/upsert_reflection_record.py`.
6. Report the final stored JSON object back to the user.

## Required Output

Always include:

- the appended JSON object (verbatim)
- chosen `level`
- top 1-2 `refs` that justify the candidate

## Hard Bans

- Do not create a candidate without `workflow`, `outputs`, and `hard_bans`.
- Do not claim a candidate is promoted; promotion must be logged separately.
- Do not invent references; if you cannot cite a source, do not record the candidate yet.

## Reference

See [references/schema.md](references/schema.md) for the exact JSON fields and constraints.
