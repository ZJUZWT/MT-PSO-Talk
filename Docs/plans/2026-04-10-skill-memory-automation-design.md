# Skill And Memory Automation Design

**Goal:** Reduce repeated forgetting by letting the agent automatically harvest durable lessons during dialogue, write stable observations into memory, and accumulate candidate skill rules that can later be promoted into formal skills.

## Problem

We are repeatedly discovering useful project rules and user preferences, but they are lost unless they are manually re-written into both the repo skill source and the runtime mirror. The result is drift:

- hard-won layout and review rules are rediscovered multiple times
- user preferences get treated like one-turn context instead of long-term constraints
- formal skill rules stay too generic because durable lessons never get promoted out of chat

The user wants this process to become automatic during conversation rather than relying on operator discipline.

## Constraints

1. Repo code cannot silently mutate Codex internal memory or skill behavior unless the agent chooses to run the appropriate tools.
2. Project skills are dual-artifact:
   - source of truth under `Docs/SkillFramework/...`
   - runtime mirror under `~/.codex/skills/...`
3. `memory` and `skill` should not have the same promotion threshold:
   - `memory` can absorb stable facts and preferences quickly
   - `skill` is executable process knowledge and must resist one-off noise
4. The user already prefers a layered model:
   - lower-level skills for single capabilities
   - higher-level skills for orchestration

## Options Considered

### Option 1: Full Auto-Write To Memory And Skill Every Turn

On every turn, classify everything interesting and directly update both memory and formal skill files.

**Pros**

- maximum automation
- no operator follow-up needed

**Cons**

- very high risk of polluting formal skills with one-off decisions
- easy to encode temporary page-specific fixes as global workflow law
- hard to audit and rollback

**Verdict:** Rejected.

### Option 2: Auto Memory + Skill Candidate Queue + Controlled Promotion

Stable lessons are split into two lanes:

- write long-lived facts/preferences into memory immediately when confidence is high
- write workflow lessons into a skill-candidate ledger first
- promote candidates into formal skills only when:
  - the user explicitly says to persist them into skill, or
  - the same lesson recurs with enough evidence

**Pros**

- preserves the speed of automatic memory capture
- protects formal skills from accidental drift
- creates an audit trail
- matches the user's layered skill philosophy

**Cons**

- slightly more moving parts
- requires a promotion protocol

**Verdict:** Approved direction.

### Option 3: Manual Only With Better Discipline

Leave the current manual process in place and rely on the agent to remember to update files and memory.

**Pros**

- simplest implementation
- lowest tooling complexity

**Cons**

- does not solve the actual forgetting problem
- depends on perfect agent behavior

**Verdict:** Rejected.

## Approved Direction

Implement a reflection pipeline with two lower-level skills and one higher-level orchestration skill.

### Lower-Level Skill 1: `capture-project-memory`

Responsibility:

- decide whether a lesson is stable enough for memory
- write the observation into Codex memory
- append an audit record into a repo-local ledger

Use cases:

- user long-term preferences
- project-specific facts
- repeatedly confirmed constraints

### Lower-Level Skill 2: `record-skill-candidate`

Responsibility:

- record workflow lessons that might deserve formal skill promotion
- dedupe by lesson fingerprint
- track evidence count, originating conversation, and promotion status

Use cases:

- new blocker rules
- new review criteria
- new mirror invariants
- new hard bans or workflow gates

### Higher-Level Skill: `conversation-lesson-harvester`

Responsibility:

- inspect the latest turn or turn cluster
- classify lessons into:
  - no-op
  - memory-only
  - skill-candidate-only
  - both
- dispatch the two lower-level skills
- promote skill candidates into formal skill docs when policy allows
- sync the updated repo skill into the runtime mirror through the existing dual-artifact workflow

## Feasibility Boundary

This design does **not** claim that local repo code can run invisibly in the background and rewrite Codex internals without agent participation.

What it **does** automate is the in-session workflow:

- the agent follows a formal reflection protocol
- the protocol writes durable information into memory
- the protocol records candidate skill deltas in repo files
- the protocol promotes qualified deltas into formal skills

This is the strongest reliable automation available within the current environment.

## Data Model

Create a repo-local ledger under `Docs/SkillFramework/automation/`:

- `memory-log.jsonl`
- `skill-candidates.jsonl`
- `promotion-log.jsonl`

Each record should include:

- `id`
- `timestamp`
- `domain`
- `summary`
- `source`
- `evidence`
- `fingerprint`
- `status`
- optional `memory_entity`
- optional `promoted_skill_path`

## Promotion Policy

### Memory Write Policy

Write directly into memory when the lesson is:

- user-specific and likely long-lived
- project-specific and repeatedly relied on
- not obviously temporary

### Skill Candidate Policy

Write into the skill-candidate ledger when the lesson changes:

- workflow sequencing
- blocker definitions
- review criteria
- hard bans
- formal mirror invariants

### Skill Promotion Policy

Promote a candidate into formal skill docs only when one of these is true:

1. the user explicitly asks to write it into skill
2. the same candidate appears at least twice with matching fingerprint
3. the same lesson is independently reinforced by both blind critics and the operator keeps using it

## Files To Add Or Modify

### New Skills

- `Docs/SkillFramework/lower-level/capture-project-memory/SKILL.md`
- `Docs/SkillFramework/lower-level/capture-project-memory/references/rules.md`
- `Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md`
- `Docs/SkillFramework/lower-level/record-skill-candidate/references/schema.md`
- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/SKILL.md`
- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/classification-rules.md`
- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/promotion-policy.md`

### New Ledgers / Automation Assets

- `Docs/SkillFramework/automation/README.md`
- `Docs/SkillFramework/automation/memory-log.jsonl`
- `Docs/SkillFramework/automation/skill-candidates.jsonl`
- `Docs/SkillFramework/automation/promotion-log.jsonl`

### New Scripts

- `scripts/skill-memory-automation/upsert_reflection_record.py`
- `scripts/skill-memory-automation/promote_skill_candidate.py`

### Existing Files To Touch

- `Docs/SkillFramework/higher-level/slide-geometry-harness/SKILL.md`
- `Docs/SkillFramework/higher-level/edge-image-aesthetic-review/SKILL.md`

These existing higher-level skills should learn when to invoke the new harvester after a durable lesson is discovered.

## Behavioral Rules

1. Never directly rewrite a formal `SKILL.md` from a one-off complaint.
2. Never skip the candidate ledger for workflow-level lessons unless the user explicitly asked for skill persistence.
3. Always keep skill source and runtime mirror aligned through the existing sync path.
4. Always leave an audit trail for memory writes and promotions.
5. Distinguish user preference from project fact from workflow rule.

## Acceptance Criteria

1. A stable user preference can be written into memory and logged in the repo without touching formal skill docs.
2. A workflow lesson can be captured as a skill candidate without immediately mutating `SKILL.md`.
3. A promoted candidate updates:
   - repo skill source
   - runtime mirror
   - promotion ledger
4. Duplicate candidate submissions are deduped by fingerprint instead of creating unbounded noise.
5. Existing project skills can opt into this harvester without changing their core job.

## Recommendation

Build the smallest useful version first:

- lower-level memory capture skill
- lower-level skill-candidate recorder
- higher-level harvester
- JSONL ledgers
- promotion script

Do **not** build a daemon, database, or background watcher in v1.
