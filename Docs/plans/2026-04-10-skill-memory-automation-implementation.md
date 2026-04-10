# Skill And Memory Automation Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a repo-local reflection pipeline that automatically writes stable lessons into memory, accumulates workflow lessons as skill candidates, and promotes approved candidates into formal project skills.

**Architecture:** Add two lower-level skills and one higher-level orchestration skill under `Docs/SkillFramework`, store candidate/audit data under `Docs/SkillFramework/automation`, and use Python helper scripts under `scripts/skill-memory-automation` for dedupe and promotion bookkeeping. Promotion into formal skill docs must reuse the existing dual-artifact sync workflow so repo skill source and `~/.codex/skills` remain aligned.

**Tech Stack:** Markdown skill docs, JSONL ledgers, Python 3 standard library scripts, existing Codex memory tools, existing dual-artifact skill sync script.

---

### Task 1: Create The Automation Ledger Surface

**Files:**
- Create: `Docs/SkillFramework/automation/README.md`
- Create: `Docs/SkillFramework/automation/memory-log.jsonl`
- Create: `Docs/SkillFramework/automation/skill-candidates.jsonl`
- Create: `Docs/SkillFramework/automation/promotion-log.jsonl`

**Step 1: Write the failing expectation**

Define the expected ledger contract in `Docs/SkillFramework/automation/README.md`:

- each file is append-only JSONL
- each record has `id`, `timestamp`, `domain`, `summary`, `source`, `fingerprint`, `status`
- promotion records must reference both source candidate and promoted skill path

**Step 2: Verify the files do not exist yet**

Run: `find Docs/SkillFramework/automation -maxdepth 1 -type f`

Expected: directory missing or files missing

**Step 3: Create the minimal ledger files**

Create the directory and the three empty `.jsonl` files plus the README with the schema summary.

**Step 4: Verify the surface exists**

Run: `find Docs/SkillFramework/automation -maxdepth 1 -type f | sort`

Expected:

- `Docs/SkillFramework/automation/README.md`
- `Docs/SkillFramework/automation/memory-log.jsonl`
- `Docs/SkillFramework/automation/skill-candidates.jsonl`
- `Docs/SkillFramework/automation/promotion-log.jsonl`

### Task 2: Add The Lower-Level Memory Capture Skill

**Files:**
- Create: `Docs/SkillFramework/lower-level/capture-project-memory/SKILL.md`
- Create: `Docs/SkillFramework/lower-level/capture-project-memory/references/rules.md`

**Step 1: Write the failing contract**

Document the required behavior:

- accept only stable observations
- classify observation as user preference, project fact, or lesson
- write to Codex memory
- append matching audit row into `Docs/SkillFramework/automation/memory-log.jsonl`
- refuse temporary or speculative observations

**Step 2: Verify the skill path is absent**

Run: `find Docs/SkillFramework/lower-level/capture-project-memory -maxdepth 3 -type f`

Expected: no output

**Step 3: Write the skill docs**

Add:

- `SKILL.md` with when-to-use, workflow, hard bans, required output
- `references/rules.md` with classification examples and reject cases

**Step 4: Verify the skill docs**

Run: `find Docs/SkillFramework/lower-level/capture-project-memory -maxdepth 3 -type f | sort`

Expected:

- `Docs/SkillFramework/lower-level/capture-project-memory/SKILL.md`
- `Docs/SkillFramework/lower-level/capture-project-memory/references/rules.md`

### Task 3: Add The Lower-Level Skill Candidate Recorder

**Files:**
- Create: `Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md`
- Create: `Docs/SkillFramework/lower-level/record-skill-candidate/references/schema.md`
- Create: `scripts/skill-memory-automation/upsert_reflection_record.py`
- Create: `scripts/skill-memory-automation/tests/test_upsert_reflection_record.py`

**Step 1: Write the failing test**

In `scripts/skill-memory-automation/tests/test_upsert_reflection_record.py`, define tests for:

- appending a new candidate record
- deduping a repeated candidate by fingerprint
- incrementing occurrence count instead of duplicating rows

Use `unittest`, `tempfile`, and a temporary JSONL file so the test does not mutate real ledgers.

**Step 2: Run test to verify it fails**

Run:

```bash
python3 -m unittest scripts.skill-memory-automation.tests.test_upsert_reflection_record
```

Expected: FAIL because the module/script does not exist yet

**Step 3: Write minimal implementation**

Implement `scripts/skill-memory-automation/upsert_reflection_record.py` using Python standard library only:

- load existing JSONL rows
- match by `fingerprint`
- append if new
- update `occurrence_count`, `last_seen_at`, and `evidence` if duplicate
- write rows back deterministically

Also add the skill docs that describe when to record a candidate and when not to.

**Step 4: Run test to verify it passes**

Run:

```bash
python3 -m unittest scripts.skill-memory-automation.tests.test_upsert_reflection_record
```

Expected: PASS

### Task 4: Add The Higher-Level Conversation Harvester

**Files:**
- Create: `Docs/SkillFramework/higher-level/conversation-lesson-harvester/SKILL.md`
- Create: `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/classification-rules.md`
- Create: `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/promotion-policy.md`

**Step 1: Write the failing contract**

Specify that the harvester must:

- inspect the latest turn or turn cluster
- classify lessons into `memory`, `skill-candidate`, `both`, or `discard`
- dispatch the lower-level memory and candidate skills
- only promote a candidate when policy allows

**Step 2: Verify the skill path is absent**

Run: `find Docs/SkillFramework/higher-level/conversation-lesson-harvester -maxdepth 3 -type f`

Expected: no output

**Step 3: Write the skill docs**

Include:

- trigger phrases such as `记住`, `写进skill`, `不要再忘`, `固化`
- promotion gates
- required audit outputs
- hard bans against direct blind mutation of `SKILL.md`

**Step 4: Verify the skill docs**

Run: `find Docs/SkillFramework/higher-level/conversation-lesson-harvester -maxdepth 3 -type f | sort`

Expected:

- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/SKILL.md`
- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/classification-rules.md`
- `Docs/SkillFramework/higher-level/conversation-lesson-harvester/references/promotion-policy.md`

### Task 5: Implement Candidate Promotion And Sync

**Files:**
- Create: `scripts/skill-memory-automation/promote_skill_candidate.py`
- Create: `scripts/skill-memory-automation/tests/test_promote_skill_candidate.py`
- Modify: `Docs/SkillFramework/higher-level/slide-geometry-harness/SKILL.md`
- Modify: `Docs/SkillFramework/higher-level/edge-image-aesthetic-review/SKILL.md`

**Step 1: Write the failing test**

In `scripts/skill-memory-automation/tests/test_promote_skill_candidate.py`, define tests for:

- selecting a candidate by id/fingerprint
- marking it promoted in `skill-candidates.jsonl`
- appending a row to `promotion-log.jsonl`
- refusing promotion if required metadata is missing

Use temporary files and do not touch the real repo ledgers in tests.

**Step 2: Run test to verify it fails**

Run:

```bash
python3 -m unittest scripts.skill-memory-automation.tests.test_promote_skill_candidate
```

Expected: FAIL because the promotion script does not exist yet

**Step 3: Write minimal implementation**

Implement `promote_skill_candidate.py` to:

- resolve the candidate
- update candidate status
- append a promotion record
- print the exact next action needed for skill-source editing/sync

Then patch the two existing higher-level skills so they explicitly invoke `conversation-lesson-harvester` after durable lessons are confirmed.

**Step 4: Run tests to verify they pass**

Run:

```bash
python3 -m unittest \
  scripts.skill-memory-automation.tests.test_upsert_reflection_record \
  scripts.skill-memory-automation.tests.test_promote_skill_candidate
```

Expected: PASS

### Task 6: Sync Runtime Mirrors And Verify End-To-End

**Files:**
- Modify: `~/.codex/skills/capture-project-memory/...` via sync
- Modify: `~/.codex/skills/record-skill-candidate/...` via sync
- Modify: `~/.codex/skills/conversation-lesson-harvester/...` via sync

**Step 1: Run the sync script**

Run:

```bash
python3 Docs/SkillFramework/lower-level/dual-artifact-skill-contract/scripts/sync_skillframework_to_codex.py
```

Expected: new runtime mirrors exist for every newly created skill and updated existing skills are refreshed

**Step 2: Verify the mirrored runtime directories**

Run:

```bash
find ~/.codex/skills -maxdepth 2 \
  \( -name 'capture-project-memory' -o -name 'record-skill-candidate' -o -name 'conversation-lesson-harvester' \) \
  -type d | sort
```

Expected: all three runtime mirror directories exist

**Step 3: Run end-to-end smoke verification**

Use one known durable lesson from the current thread:

- user preference about preserving formal semantic lanes
- page09 `PSO Cache` requiring visible `VS Hash` and `PS Hash`

Manually walk the workflow:

- classify
- write memory
- write skill candidate
- promote if explicit
- verify ledger rows were appended

**Step 4: Commit**

```bash
git add Docs/SkillFramework scripts/skill-memory-automation Docs/plans
git commit -m "feat: add skill and memory reflection automation"
```
