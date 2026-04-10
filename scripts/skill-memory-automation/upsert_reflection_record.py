#!/usr/bin/env python3
"""
Deterministic JSONL upsert for reflection/candidate ledgers.

This script upserts a single JSON record into a JSONL ledger by matching on an
explicit key field/value. If a match exists, it updates:
  - occurrence_count: incremented by 1
  - last_seen_at: set from incoming last_seen_at, else incoming timestamp
  - evidence: merged (preserving order, de-duping exact string entries)

All writes are deterministic (sorted keys, compact separators) and atomic.
Python 3 standard library only.
"""

from __future__ import annotations

import argparse
import json
import os
import tempfile
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


def _normalize_evidence(value: Any) -> List[str]:
    if value is None:
        return []
    if isinstance(value, str):
        v = value.strip()
        return [v] if v else []
    if isinstance(value, list):
        out: List[str] = []
        for item in value:
            if isinstance(item, str):
                s = item.strip()
                if s:
                    out.append(s)
        return out
    return []


def _merge_evidence(existing: Any, incoming: Any) -> Any:
    existing_list = _normalize_evidence(existing)
    incoming_list = _normalize_evidence(incoming)
    if not existing_list and not incoming_list:
        return existing if existing is not None else incoming
    seen = set()
    merged: List[str] = []
    for s in existing_list + incoming_list:
        if s not in seen:
            seen.add(s)
            merged.append(s)
    return merged


def _get_last_seen_from_incoming(incoming: Dict[str, Any]) -> Optional[str]:
    v = incoming.get("last_seen_at")
    if isinstance(v, str) and v.strip():
        return v.strip()
    ts = incoming.get("timestamp")
    if isinstance(ts, str) and ts.strip():
        return ts.strip()
    return None


def upsert_in_rows(
    *,
    rows: List[Dict[str, Any]],
    match_field: str,
    match_value: str,
    incoming: Dict[str, Any],
    update_only: bool,
) -> Tuple[str, Dict[str, Any], List[Dict[str, Any]]]:
    if match_field not in ("id", "fingerprint"):
        raise ValueError("match_field must be 'id' or 'fingerprint'")
    if not match_value:
        raise ValueError("match_value must be non-empty")

    if match_field not in incoming:
        raise ValueError(f"incoming record missing required match field '{match_field}'")
    if str(incoming.get(match_field)) != match_value:
        raise ValueError(
            f"incoming record {match_field} mismatch: "
            f"{incoming.get(match_field)!r} != {match_value!r}"
        )

    matches = [i for i, r in enumerate(rows) if str(r.get(match_field, "")) == match_value]
    if len(matches) > 1:
        raise ValueError(f"Multiple rows match {match_field}={match_value!r}")

    if not matches:
        if update_only:
            raise ValueError(f"No existing row matches {match_field}={match_value!r}")
        # Ensure minimal bookkeeping fields exist for new rows.
        out = dict(incoming)
        if "occurrence_count" not in out:
            out["occurrence_count"] = 1
        last_seen = _get_last_seen_from_incoming(out)
        if last_seen is not None:
            out["last_seen_at"] = last_seen
        # Normalize evidence shape deterministically.
        if "evidence" in out:
            out["evidence"] = _merge_evidence(None, out.get("evidence"))
        rows.append(out)
        return ("inserted", out, rows)

    idx = matches[0]
    existing = rows[idx]

    # Prevent silent "same fingerprint, different id" collisions.
    if "id" in existing and "id" in incoming:
        if isinstance(existing.get("id"), str) and isinstance(incoming.get("id"), str):
            if existing["id"] and incoming["id"] and existing["id"] != incoming["id"]:
                raise ValueError(
                    f"Incoming id {incoming['id']!r} conflicts with existing id {existing['id']!r} "
                    f"for {match_field}={match_value!r}"
                )

    updated: Dict[str, Any] = dict(existing)
    # Default behavior: overlay incoming fields.
    for k, v in incoming.items():
        updated[k] = v

    # occurrence_count increments per successful dedupe/upsert call.
    try:
        prev_count = int(existing.get("occurrence_count", 1))
    except (TypeError, ValueError):
        prev_count = 1
    updated["occurrence_count"] = prev_count + 1

    last_seen = _get_last_seen_from_incoming(incoming)
    if last_seen is not None:
        updated["last_seen_at"] = last_seen

    if "evidence" in incoming or "evidence" in existing:
        updated["evidence"] = _merge_evidence(existing.get("evidence"), incoming.get("evidence"))

    rows[idx] = updated
    return ("updated", updated, rows)


def main(argv: Optional[List[str]] = None) -> int:
    p = argparse.ArgumentParser(description="Upsert a JSON record into a JSONL ledger deterministically.")
    p.add_argument("--ledger", required=True, help="Path to JSONL ledger to upsert into.")
    p.add_argument(
        "--match-field",
        required=True,
        choices=["id", "fingerprint"],
        help="Field to match on for upsert.",
    )
    p.add_argument("--match-value", required=True, help="Value of match field to upsert by.")
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--record-json", help="Incoming record as a JSON object string.")
    g.add_argument("--record-path", help="Path to a JSON file containing the incoming object record.")
    p.add_argument(
        "--update-only",
        action="store_true",
        help="Fail if the record does not already exist (do not insert).",
    )

    args = p.parse_args(argv)

    if args.record_json is not None:
        try:
            incoming = json.loads(args.record_json)
        except json.JSONDecodeError as e:
            p.error(f"--record-json is not valid JSON: {e}")
        if not isinstance(incoming, dict):
            p.error("--record-json must be a JSON object")
    else:
        try:
            with open(args.record_path, "r", encoding="utf-8") as f:
                incoming = json.load(f)
        except OSError as e:
            p.error(f"Failed to read --record-path: {e}")
        except json.JSONDecodeError as e:
            p.error(f"--record-path did not contain valid JSON: {e}")
        if not isinstance(incoming, dict):
            p.error("--record-path JSON must be an object")

    try:
        rows = _read_jsonl(args.ledger)
        action, record, rows = upsert_in_rows(
            rows=rows,
            match_field=args.match_field,
            match_value=args.match_value,
            incoming=incoming,
            update_only=args.update_only,
        )
    except ValueError as e:
        print(f"ERROR: {e}")
        return 2

    out_text = "".join(_json_dumps_deterministic(r) + "\n" for r in rows)
    _atomic_write_text(args.ledger, out_text)

    print(_json_dumps_deterministic({"result": action, "match_field": args.match_field, "match_value": args.match_value}))
    print(_json_dumps_deterministic({"record": record}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
