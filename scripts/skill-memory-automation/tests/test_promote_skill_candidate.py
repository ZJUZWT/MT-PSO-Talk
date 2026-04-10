import json
import os
import subprocess
import sys
import tempfile
import unittest


def _script_path() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(here, "..", "promote_skill_candidate.py"))


def _run(args):
    return subprocess.run(
        [sys.executable, _script_path(), *args],
        text=True,
        capture_output=True,
    )


def _write_jsonl(path: str, rows):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for r in rows:
            f.write(json.dumps(r, sort_keys=True, ensure_ascii=True, separators=(",", ":")) + "\n")


def _read_jsonl(path: str):
    if not os.path.exists(path):
        return []
    out = []
    with open(path, "r", encoding="utf-8") as f:
        for ln in f:
            ln = ln.strip()
            if ln:
                out.append(json.loads(ln))
    return out


class TestPromoteSkillCandidate(unittest.TestCase):
    def test_promote_by_id_marks_promoted_and_appends_log(self):
        with tempfile.TemporaryDirectory() as td:
            candidates = os.path.join(td, "skill-candidates.jsonl")
            promolog = os.path.join(td, "promotion-log.jsonl")
            rows = [
                {
                    "id": "c1",
                    "timestamp": "2026-04-10T00:00:00Z",
                    "domain": "workflow",
                    "summary": "Use subprocess tests for hyphenated dirs.",
                    "source": "thread:test",
                    "evidence": "plan requirement",
                    "fingerprint": "fp-1",
                    "status": "candidate",
                },
                {
                    "id": "c2",
                    "timestamp": "2026-04-10T00:10:00Z",
                    "domain": "workflow",
                    "summary": "Another candidate",
                    "source": "thread:test",
                    "evidence": ["a"],
                    "fingerprint": "fp-2",
                    "status": "candidate",
                },
            ]
            _write_jsonl(candidates, rows)

            cp = _run(
                [
                    "--candidates-ledger",
                    candidates,
                    "--promotion-log",
                    promolog,
                    "--select-field",
                    "id",
                    "--select-value",
                    "c1",
                    "--promoted-skill-path",
                    "Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md",
                    "--promoted-at",
                    "2026-04-10T02:03:04Z",
                ]
            )
            self.assertEqual(cp.returncode, 0, msg=f"stderr:\n{cp.stderr}\nstdout:\n{cp.stdout}")

            updated = _read_jsonl(candidates)
            self.assertEqual(len(updated), 2)
            c1 = next(r for r in updated if r["id"] == "c1")
            self.assertEqual(c1["status"], "promoted")
            self.assertEqual(c1["promoted_at"], "2026-04-10T02:03:04Z")
            self.assertEqual(
                c1["promoted_skill_path"],
                "Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md",
            )
            c2 = next(r for r in updated if r["id"] == "c2")
            self.assertEqual(c2["status"], "candidate")

            logs = _read_jsonl(promolog)
            self.assertEqual(len(logs), 1)
            log = logs[0]
            self.assertEqual(log["status"], "promoted")
            self.assertEqual(log["source_candidate_id"], "c1")
            self.assertEqual(log["source_candidate_timestamp"], "2026-04-10T00:00:00Z")
            self.assertEqual(log["promoted_skill_path"], "Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md")

    def test_refuse_promotion_if_required_metadata_missing(self):
        with tempfile.TemporaryDirectory() as td:
            candidates = os.path.join(td, "skill-candidates.jsonl")
            promolog = os.path.join(td, "promotion-log.jsonl")
            rows = [
                {
                    "id": "c1",
                    "timestamp": "2026-04-10T00:00:00Z",
                    "domain": "workflow",
                    "summary": "",  # invalid
                    "source": "thread:test",
                    "evidence": "plan requirement",
                    "fingerprint": "fp-1",
                    "status": "candidate",
                }
            ]
            _write_jsonl(candidates, rows)
            with open(candidates, "r", encoding="utf-8") as f:
                before = f.read()

            cp = _run(
                [
                    "--candidates-ledger",
                    candidates,
                    "--promotion-log",
                    promolog,
                    "--select-field",
                    "id",
                    "--select-value",
                    "c1",
                    "--promoted-skill-path",
                    "Docs/SkillFramework/lower-level/record-skill-candidate/SKILL.md",
                    "--promoted-at",
                    "2026-04-10T02:03:04Z",
                ]
            )
            self.assertNotEqual(cp.returncode, 0, msg="Expected failure")
            with open(candidates, "r", encoding="utf-8") as f:
                after = f.read()
            self.assertEqual(before, after, msg="Ledger should not change on failure")
            self.assertFalse(os.path.exists(promolog), msg="Promotion log should not be created on failure")


if __name__ == "__main__":
    unittest.main()
