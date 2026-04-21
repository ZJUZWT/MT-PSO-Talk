from __future__ import annotations

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


def load_audit_storyboard_sync_module():
    repo_root = Path(__file__).resolve().parents[3]
    module_path = repo_root / "scripts/slide-geometry-harness/audit_storyboard_sync.py"
    spec = importlib.util.spec_from_file_location("audit_storyboard_sync", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load spec for {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class AuditStoryboardSyncTests(unittest.TestCase):
    def test_iter_story_docs_keeps_opening_page_but_skips_audit_artifact(self):
        module = load_audit_storyboard_sync_module()

        with tempfile.TemporaryDirectory() as temp_dir:
            story_dir = Path(temp_dir)
            opening_doc = story_dir / "00-开场问题页.md"
            audit_doc = story_dir / "00-剧本与页面对齐审计.md"
            page_doc = story_dir / "01-第一页.md"

            for path in (opening_doc, audit_doc, page_doc):
                path.write_text("# mock\n", encoding="utf-8")

            docs = list(module.iter_story_docs(story_dir))
            doc_names = [doc.path.name for doc in docs]

            self.assertIn("00-开场问题页.md", doc_names)
            self.assertIn("01-第一页.md", doc_names)
            self.assertNotIn("00-剧本与页面对齐审计.md", doc_names)


if __name__ == "__main__":
    unittest.main()
