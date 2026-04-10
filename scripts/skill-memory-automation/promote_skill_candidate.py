#!/usr/bin/env python3
"""
Promote a skill candidate from a JSONL ledger.

Behavior:
- Select a candidate by explicit (id|fingerprint) selector.
- Validate required metadata exists on the candidate.
- Refuse if the candidate is already promoted.
- Mark candidate as promoted in the candidates ledger.
- Append a promotion record to a promotion-log JSONL file.
- Print a concrete "next action" message for manual skill-source editing + sync.

All JSON writes are deterministic and atomic.
Python 3 standard library only.
"""

from __future__ import annotations

import argparse
import json
import os
import tempfile
import uuid
from typing import Any, Dict, List, Optional, Tuple


def _json_dumps_deterministic(obj: Any) -> str:
    return json.dumps(obj, sort_keys=True, ensure_ascii=True, separators=(",", ":"))


def _read_jsonl(path: str) -> List[Dict[str, Any]]:
    if not os.path.exists(path):
        return []
    rows: List[Dict[str, Any]] = []
    with open(path, "r", encoding="utf-8") as f:
        for lineno, raw in enumerate(f, start=1):
            line = raw.strip()
            if not line:
                continue
            try:
                obj = json.loads(line)
            except json.JSONDecodeError as e:
                raise ValueError(f"Invalid JSON on line {lineno} of {path}: {e}") from e
            if not isinstance(obj, dict):
                raise ValueError(f"Non-object JSON on line {lineno} of {path}")
            rows.append(obj)
    return rows


def _atomic_write_text(path: str, text: str) -> None:
    parent = os.path.dirname(os.path.abspath(path)) or "."
    os.makedirs(parent, exist_ok=True)
    fd, tmppath = tempfile.mkstemp(prefix=".tmp-", suffix=".jsonl", dir=parent)
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        os.replace(tmppath, path)
    finally:
        try:
            if os.path.exists(tmppath):
                os.unlink(tmppath)
        except OSError:
            pass


def _append_jsonl_row(path: str, row: Dict[str, Any]) -> None:
    parent = os.path.dirname(os.path.abspath(path)) or "."
    os.makedirs(parent, exist_ok=True)
    line = _json_dumps_deterministic(row) + "\n"
    with open(path, "a", encoding="utf-8", newline="\n") as f:
        f.write(line)


def _require_nonempty_str(rec: Dict[str, Any], key: str) -> str:
    v = rec.get(key)
    if not isinstance(v, str) or not v.strip():
        raise ValueError(f"Candidate missing required non-empty '{key}'")
    return v.strip()


def _require_evidence(rec: Dict[str, Any]) -> Any:
    if "evidence" not in rec:
        raise ValueError("Candidate missing required 'evidence'")
    ev = rec.get("evidence")
    if isinstance(ev, str):
        if not ev.strip():
            raise ValueError("Candidate has empty 'evidence'")
        return ev.strip()
    if isinstance(ev, list):
        cleaned = [x.strip() for x in ev if isinstance(x, str) and x.strip()]
        if not cleaned:
            raise ValueError("Candidate has empty 'evidence'")
        return cleaned
    raise ValueError("Candidate 'evidence' must be a string or list of strings")


def _select_candidate(
    rows: List[Dict[str, Any]], select_field: str, select_value: str
) -> Tuple[int, Dict[str, Any]]:
    matches = [i for i, r in enumerate(rows) if str(r.get(select_field, "")) == select_value]
    if not matches:
        raise ValueError(f"No candidate found with {select_field}={select_value!r}")
    if len(matches) > 1:
        raise ValueError(f"Multiple candidates found with {select_field}={select_value!r}")
    idx = matches[0]
    return idx, rows[idx]


def promote_candidate(
    *,
    candidates_rows: List[Dict[str, Any]],
    select_field: str,
    select_value: str,
    promoted_skill_path: str,
    promoted_at: str,
) -> Tuple[Dict[str, Any], Dict[str, Any], List[Dict[str, Any]]]:
    if select_field not in ("id", "fingerprint"):
        raise ValueError("select_field must be 'id' or 'fingerprint'")
    if not select_value:
        raise ValueError("select_value must be non-empty")
    if not promoted_skill_path:
        raise ValueError("promoted_skill_path must be non-empty")
    if not promoted_at:
        raise ValueError("promoted_at must be non-empty")

    idx, cand = _select_candidate(candidates_rows, select_field, select_value)

    status = cand.get("status")
    if isinstance(status, str) and status.strip().lower() == "promoted":
        raise ValueError("Candidate is already promoted")

    # Validate required metadata (per plan docs).
    cand_id = _require_nonempty_str(cand, "id")
    cand_ts = _require_nonempty_str(cand, "timestamp")
    cand_domain = _require_nonempty_str(cand, "domain")
    cand_summary = _require_nonempty_str(cand, "summary")
    cand_source = _require_nonempty_str(cand, "source")
    cand_fp = _require_nonempty_str(cand, "fingerprint")
    cand_evidence = _require_evidence(cand)

    updated = dict(cand)
    updated["status"] = "promoted"
    updated["promoted_at"] = promoted_at
    updated["promoted_skill_path"] = promoted_skill_path

    candidates_rows[idx] = updated

    promotion_record: Dict[str, Any] = {
        "id": str(uuid.uuid4()),
        "timestamp": promoted_at,
        "status": "promoted",
        "domain": cand_domain,
        "summary": cand_summary,
        "source": cand_source,
        "evidence": cand_evidence,
        "fingerprint": cand_fp,
        "source_candidate_id": cand_id,
        "source_candidate_timestamp": cand_ts,
        "promoted_skill_path": promoted_skill_path,
    }

    return updated, promotion_record, candidates_rows


def main(argv: Optional[List[str]] = None) -> int:
    p = argparse.ArgumentParser(description="Promote a skill candidate and append a promotion log row.")
    p.add_argument("--candidates-ledger", required=True, help="Path to skill-candidates.jsonl")
    p.add_argument("--promotion-log", required=True, help="Path to promotion-log.jsonl")
    p.add_argument("--select-field", required=True, choices=["id", "fingerprint"], help="Field to select candidate by")
    p.add_argument("--select-value", required=True, help="Value of select field")
    p.add_argument("--promoted-skill-path", required=True, help="Target skill source path to edit (recorded in logs)")
    p.add_argument(
        "--promoted-at",
        required=True,
        help="Promotion timestamp (ISO-8601 string recommended). Must be provided explicitly for determinism.",
    )

    args = p.parse_args(argv)

    try:
        candidates_rows = _read_jsonl(args.candidates_ledger)
        updated_candidate, promotion_record, updated_rows = promote_candidate(
            candidates_rows=candidates_rows,
            select_field=args.select_field,
            select_value=args.select_value,
            promoted_skill_path=args.promoted_skill_path,
            promoted_at=args.promoted_at,
        )
    except ValueError as e:
        print(f"ERROR: {e}")
        return 2

    # Write candidates ledger atomically.
    out_candidates = "".join(_json_dumps_deterministic(r) + "\n" for r in updated_rows)
    _atomic_write_text(args.candidates_ledger, out_candidates)

    # Append promotion record.
    _append_jsonl_row(args.promotion_log, promotion_record)

    print(_json_dumps_deterministic({"result": "promoted"}))
    print(_json_dumps_deterministic({"candidate": updated_candidate}))
    print(_json_dumps_deterministic({"promotion_record": promotion_record}))
    print(
        "NEXT: Apply the lesson into the skill source at "
        + args.promoted_skill_path
        + " then run the existing dual-artifact sync to refresh ~/.codex/skills."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

