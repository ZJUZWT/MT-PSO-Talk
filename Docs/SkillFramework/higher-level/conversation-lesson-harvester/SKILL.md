---
name: conversation-lesson-harvester
description: Use when a conversation produced multiple reusable lessons and you need to extract, classify, and log them as memory entries and/or skill candidates with clear promotion decisions.
---

# Conversation Lesson Harvester

This skill turns one conversation into a small set of durable artifacts:

- project memory entries (`memory-log.jsonl`)
- skill candidates (`skill-candidates.jsonl`)
- promotion decisions (`promotion-log.jsonl`) when warranted

It is an orchestration skill that composes:

- `capture-project-memory`
- `record-skill-candidate`

## When to Use

Use this skill when:

- a thread contains multiple concrete rules and repeatable workflows
- you need to avoid losing decisions made in the conversation
- you want consistent classification and promotion criteria

Do not use this skill for single, obvious memories or a single obvious candidate; use the lower-level skills directly.

## Required Workflow

1. Identify the conversation slice to harvest (what messages or decision block).
2. Extract candidate lessons as a short list of statements (no prose).
3. Classify each statement using [references/classification-rules.md](references/classification-rules.md) as one of:
   - `memory`
   - `skill-candidate`
   - `ignore`
4. For each `memory`, run `capture-project-memory`.
5. For each `skill-candidate`, run `record-skill-candidate`.
6. Evaluate each recorded candidate against [references/promotion-policy.md](references/promotion-policy.md).
7. If a candidate meets the threshold and is ready to land now, run `scripts/skill-memory-automation/promote_skill_candidate.py` and log the promotion event.
8. Report a concise harvest summary: counts and ids.

## Required Output

Always include:

- `Harvest scope` (what was harvested)
- `Memory entries appended` (count and candidate summary)
- `Skill candidates appended` (count and candidate ids)
- `Promotion decisions` (promoted/rejected/deferred with candidate ids)

## Hard Bans

- Do not speculate beyond what the conversation actually established.
- Do not promote without logging to `promotion-log.jsonl`.
- Do not create a skill candidate without `workflow`, `outputs`, `hard_bans`, and `refs`.
- Do not convert unstable one-offs into memory or skills.
- Do not promote a candidate without also updating the candidate ledger status.

## Reference

Use [references/classification-rules.md](references/classification-rules.md) for classification and [references/promotion-policy.md](references/promotion-policy.md) for promotion thresholds.
