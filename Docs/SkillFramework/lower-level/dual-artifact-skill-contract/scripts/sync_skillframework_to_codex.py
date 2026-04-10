#!/usr/bin/env python3

from __future__ import annotations

import argparse
import os
import shutil
import sys
from pathlib import Path

LAYERS = ("lower-level", "higher-level")


def find_repo_root(start: Path) -> Path:
    for candidate in (start, *start.parents):
        if (candidate / "Docs" / "SkillFramework").is_dir():
            return candidate

    raise FileNotFoundError(
        "Could not find repo root containing Docs/SkillFramework from "
        f"{start}"
    )


def codex_skills_root() -> Path:
    codex_home = os.environ.get("CODEX_HOME")
    if codex_home:
        return Path(codex_home).expanduser() / "skills"

    return Path.home() / ".codex" / "skills"


def declared_skill_name(skill_md: Path) -> str | None:
    for line in skill_md.read_text(encoding="utf-8").splitlines()[:16]:
        if line.startswith("name:"):
            return line.split(":", 1)[1].strip()

    return None


def iter_skill_dirs(source_root: Path):
    for layer in LAYERS:
        layer_root = source_root / layer
        if not layer_root.is_dir():
            continue

        for skill_dir in sorted(path for path in layer_root.iterdir() if path.is_dir()):
            skill_md = skill_dir / "SKILL.md"
            if not skill_md.is_file():
                continue

            yield layer, skill_dir


def sync_skill(skill_dir: Path, dest_root: Path, dry_run: bool) -> Path:
    destination = dest_root / skill_dir.name
    if dry_run:
        return destination

    if destination.exists():
        shutil.rmtree(destination)

    shutil.copytree(
        skill_dir,
        destination,
        ignore=shutil.ignore_patterns(".DS_Store", "__pycache__"),
    )
    return destination


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Mirror repo skill sources from Docs/SkillFramework into "
            "~/.codex/skills."
        )
    )
    parser.add_argument(
        "--repo-root",
        type=Path,
        help="Optional repo root override. Defaults to searching upward from cwd.",
    )
    parser.add_argument(
        "--skill",
        action="append",
        dest="skills",
        help="Sync only the named skill. Can be passed multiple times.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Print the sync plan without copying files.",
    )
    args = parser.parse_args()

    repo_root = args.repo_root.resolve() if args.repo_root else find_repo_root(Path.cwd())
    source_root = repo_root / "Docs" / "SkillFramework"
    dest_root = codex_skills_root()
    selected = set(args.skills or [])

    if not args.dry_run:
        dest_root.mkdir(parents=True, exist_ok=True)

    synced = []
    for layer, skill_dir in iter_skill_dirs(source_root):
        if selected and skill_dir.name not in selected:
            continue

        declared = declared_skill_name(skill_dir / "SKILL.md")
        if declared != skill_dir.name:
            print(
                f"error: folder '{skill_dir.name}' does not match declared "
                f"skill name '{declared}' in {skill_dir / 'SKILL.md'}",
                file=sys.stderr,
            )
            return 1

        destination = sync_skill(skill_dir, dest_root, args.dry_run)
        synced.append((layer, skill_dir, destination))

    if selected and not synced:
        print(
            "error: no skills matched selection: "
            + ", ".join(sorted(selected)),
            file=sys.stderr,
        )
        return 1

    for layer, skill_dir, destination in synced:
        status = "would sync" if args.dry_run else "synced"
        print(f"{status}: {layer}/{skill_dir.name} -> {destination}")

    print(f"done: {len(synced)} skill(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
