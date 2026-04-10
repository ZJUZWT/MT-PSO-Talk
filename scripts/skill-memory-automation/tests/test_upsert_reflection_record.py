import json
import os
import subprocess
import sys
import tempfile
import unittest


def _script_path() -> str:
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.abspath(os.path.join(here, "..", "upsert_reflection_record.py"))


def _run(args):
    return subprocess.run(
        [sys.executable, _script_path(), *args],
        text=True,
        capture_output=True,
    )


class TestUpsertReflectionRecord(unittest.TestCase):
    def test_append_new_candidate(self):
        with tempfile.TemporaryDirectory() as td:
            ledger = os.path.join(td, "skill-candidates.jsonl")
            rec = {
                "id": "c1",
                "timestamp": "2026-04-10T00:00:00Z",
                "domain": "slide-geometry",
                "summary": "Keep PSO cache visible as co-main route.",
                "source": "thread:test",
                "evidence": "page09 contract",
                "fingerprint": "fp-1",
                "status": "candidate",
            }
            cp = _run(
                [
                    "--ledger",
                    ledger,
                    "--match-field",
                    "fingerprint",
                    "--match-value",
                    "fp-1",
                    "--record-json",
                    json.dumps(rec),
                ]
            )
            self.assertEqual(cp.returncode, 0, msg=f"stderr:\n{cp.stderr}\nstdout:\n{cp.stdout}")

            with open(ledger, "r", encoding="utf-8") as f:
                lines = [ln.strip() for ln in f.readlines() if ln.strip()]
            self.assertEqual(len(lines), 1)
            row = json.loads(lines[0])
            self.assertEqual(row["fingerprint"], "fp-1")
            self.assertEqual(row["occurrence_count"], 1)
            self.assertEqual(row["last_seen_at"], "2026-04-10T00:00:00Z")
            self.assertEqual(row["evidence"], ["page09 contract"])

    def test_dedupe_by_fingerprint_increments_occurrence_and_merges_evidence(self):
        with tempfile.TemporaryDirectory() as td:
            ledger = os.path.join(td, "skill-candidates.jsonl")
            rec1 = {
                "id": "c1",
                "timestamp": "2026-04-10T00:00:00Z",
                "domain": "slide-geometry",
                "summary": "Keep PSO cache visible as co-main route.",
                "source": "thread:test",
                "evidence": "first",
                "fingerprint": "fp-1",
                "status": "candidate",
            }
            rec2 = {
                "id": "c1",
                "timestamp": "2026-04-10T01:00:00Z",
                "domain": "slide-geometry",
                "summary": "Keep PSO cache visible as co-main route.",
                "source": "thread:test",
                "evidence": ["first", "second"],
                "fingerprint": "fp-1",
                "status": "candidate",
            }

            cp1 = _run(
                [
                    "--ledger",
                    ledger,
                    "--match-field",
                    "fingerprint",
                    "--match-value",
                    "fp-1",
                    "--record-json",
                    json.dumps(rec1),
                ]
            )
            self.assertEqual(cp1.returncode, 0, msg=f"stderr:\n{cp1.stderr}\nstdout:\n{cp1.stdout}")

            cp2 = _run(
                [
                    "--ledger",
                    ledger,
                    "--match-field",
                    "fingerprint",
                    "--match-value",
                    "fp-1",
                    "--record-json",
                    json.dumps(rec2),
                ]
            )
            self.assertEqual(cp2.returncode, 0, msg=f"stderr:\n{cp2.stderr}\nstdout:\n{cp2.stdout}")

            with open(ledger, "r", encoding="utf-8") as f:
                lines = [ln.strip() for ln in f.readlines() if ln.strip()]
            self.assertEqual(len(lines), 1)
            row = json.loads(lines[0])
            self.assertEqual(row["occurrence_count"], 2)
            self.assertEqual(row["last_seen_at"], "2026-04-10T01:00:00Z")
            self.assertEqual(row["evidence"], ["first", "second"])

    def test_upsert_by_id(self):
        with tempfile.TemporaryDirectory() as td:
            ledger = os.path.join(td, "memory-log.jsonl")
            rec = {
                "id": "m1",
                "timestamp": "2026-04-10T00:00:00Z",
                "domain": "prefs",
                "summary": "Keep page09 as a single convergence page.",
                "source": "thread:test",
                "evidence": "user instruction",
                "fingerprint": "fp-m1",
                "status": "memory",
            }

            cp = _run(
                [
                    "--ledger",
                    ledger,
                    "--match-field",
                    "id",
                    "--match-value",
                    "m1",
                    "--record-json",
                    json.dumps(rec),
                ]
            )
            self.assertEqual(cp.returncode, 0, msg=f"stderr:\n{cp.stderr}\nstdout:\n{cp.stdout}")

            with open(ledger, "r", encoding="utf-8") as f:
                lines = [ln.strip() for ln in f.readlines() if ln.strip()]
            self.assertEqual(len(lines), 1)
            row = json.loads(lines[0])
            self.assertEqual(row["id"], "m1")
            self.assertEqual(row["occurrence_count"], 1)


if __name__ == "__main__":
    unittest.main()

