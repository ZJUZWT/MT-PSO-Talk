from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


def load_workflow_gate_module():
    repo_root = Path(__file__).resolve().parents[3]
    module_path = repo_root / "scripts/slide-geometry-harness/workflow_gate.py"
    spec = importlib.util.spec_from_file_location("workflow_gate", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load spec for {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class WorkflowGateTests(unittest.TestCase):
    def test_classifies_remotion_animation_work_as_full(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="把 page_31 的过渡动画和 layout 再调一下",
            files=["SlideApp/src/remotion/embed.ts"],
        )

        self.assertEqual(decision.level, "full")
        self.assertIn("timing audit", " ".join(decision.required_checks).lower())
        self.assertIn("mechanical review", " ".join(decision.required_checks).lower())

    def test_classifies_notes_copy_work_as_lite(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="修改 page_31 左边卡片文案，补一点 harness 的理解",
            files=["SlideApp/src/storyboard-data/pso-storyboard.ts"],
        )

        self.assertEqual(decision.level, "lite")
        self.assertNotIn("timing audit", " ".join(decision.required_checks).lower())
        self.assertIn(
            "python3 scripts/slide-geometry-harness/audit_storyboard_sync.py",
            decision.commands,
        )

    def test_classifies_unrelated_work_as_skip(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="整理一下 benchmark 的 README",
            files=["BenchmarkApp/README.md"],
        )

        self.assertEqual(decision.level, "skip")
        self.assertEqual(decision.commands, [])

    def test_classifies_chinese_page_layout_feedback_as_full_without_file_hints(self):
        module = load_workflow_gate_module()

        prompts = [
            "page 24，感觉也可以修改成page 26的那种左右两半方式。主要我感觉page25上面的那个左边的图很好",
            "你不要在正式的图里，放一些随意的文本。。。左边是包体，展示压缩的数据，右边是内存，展示现在左边的内容加上，然后上下可以拉高啊，你位置不够可以全部拉高",
            "包体和内存的字更大一点，上下可以更宽啊？包体你要说明是对shadercode的压缩，内存你要说明PSO在UE里面有这样的策略LRU+mmap",
        ]

        for prompt in prompts:
            with self.subTest(prompt=prompt):
                decision = module.classify_workflow(
                    prompt=prompt,
                    files=[],
                )

                self.assertEqual(decision.level, "full")
                self.assertIn(
                    "Run transition timing audit before claiming done.",
                    decision.required_checks,
                )
                self.assertIn(
                    "Run formal-page mechanical review summary before claiming done.",
                    decision.required_checks,
                )

    def test_persists_state_and_builds_stop_plan(self):
        module = load_workflow_gate_module()

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            state_path = temp_root / "gate-state.json"

            decision = module.classify_workflow(
                prompt="调整 page_31 的动画 timing",
                files=["Docs/剧本/31-第三十一页-正式动画.md"],
            )
            module.write_state(state_path, decision)

            saved_payload = json.loads(state_path.read_text(encoding="utf-8"))
            self.assertEqual(saved_payload["level"], "full")

            stop_plan = module.load_state(state_path)
            commands = module.build_stop_plan(stop_plan.level, temp_root)

            self.assertEqual(len(commands), 2)
            self.assertTrue(commands[0].endswith("audit_transition_timings.py"))
            self.assertTrue(commands[1].endswith("audit_storyboard_sync.py"))

    def test_builds_codex_user_prompt_submit_context_for_lite_work(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="修改 page_31 左边卡片文案，补一点 harness 的理解",
            files=[],
        )
        context = module.build_codex_user_prompt_submit_context(
            decision,
            Path(".git/slide-geometry-harness-workflow-state.json"),
        )

        self.assertIn("lite", context)
        self.assertIn("audit_storyboard_sync.py", context)
        self.assertIn("Stop hook", context)

    def test_builds_codex_stop_block_output_on_first_failure(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="调整 page_31 的动画 timing",
            files=["Docs/剧本/31-第三十一页-正式动画.md"],
        )
        response = module.build_codex_stop_output(
            decision,
            [
                {
                    "command": "python3 scripts/slide-geometry-harness/audit_transition_timings.py",
                    "returncode": 1,
                    "stdout": "timing audit failed",
                    "stderr": "",
                }
            ],
            stop_hook_active=False,
        )

        self.assertEqual(response["decision"], "block")
        self.assertIn("audit_transition_timings.py", response["reason"])
        self.assertIn("rerun the required audits", response["reason"])

    def test_builds_codex_stop_terminal_output_after_second_failure(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="调整 page_31 的动画 timing",
            files=["Docs/剧本/31-第三十一页-正式动画.md"],
        )
        response = module.build_codex_stop_output(
            decision,
            [
                {
                    "command": "python3 scripts/slide-geometry-harness/audit_transition_timings.py",
                    "returncode": 1,
                    "stdout": "",
                    "stderr": "still failing",
                }
            ],
            stop_hook_active=True,
        )

        self.assertFalse(response["continue"])
        self.assertIn("still failing", response["stopReason"])
        self.assertIn("audit_transition_timings.py", response["systemMessage"])

    def test_builds_mechanical_review_command_for_full_work(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="调整 page_31 的动画 timing",
            files=["Docs/剧本/31-第三十一页-正式动画.md"],
        )
        command = module.build_mechanical_review_command(decision)

        self.assertIsNotNone(command)
        self.assertIn("--silent", command)
        self.assertIn("review:mechanical", command)
        self.assertIn("--from page_31", command)

    def test_builds_mechanical_review_command_from_earliest_affected_step(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="page 02 左上 PSO 卡和 GPU 重叠了",
            files=["SlideApp/src/remotion/pages/Page02Scene.tsx"],
        )

        self.assertEqual(decision.mechanical_review_from_step, "page_02")
        self.assertEqual(decision.focus_step_ids, ["page_02"])
        self.assertIn(
            "--from page_02",
            module.build_mechanical_review_command(decision),
        )

    def test_blocks_when_mechanical_review_reports_blocker_pages(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="调整 page_31 的动画 timing",
            files=["Docs/剧本/31-第三十一页-正式动画.md"],
        )
        response = module.build_codex_stop_output(
            decision,
            [],
            stop_hook_active=False,
            review_summary={
                "has_blocker_pages": True,
                "blocker_pages": [
                    {
                        "stepId": "page_31",
                        "reviewSource": "formal",
                        "mechanicalScore": "5.8/10",
                        "verdict": "Open the layout before critic pass",
                        "topFixes": [
                            "Increase the gap between the chain and rules columns."
                        ],
                    }
                ],
            },
        )

        self.assertEqual(response["decision"], "block")
        self.assertIn("page_31", response["reason"])
        self.assertIn("formal", response["reason"])
        self.assertIn("Open the layout before critic pass", response["reason"])

    def test_blocks_when_focus_step_is_missing_review_surface(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="page 02 左上 PSO 卡和 GPU 重叠了",
            files=["SlideApp/src/remotion/pages/Page02Scene.tsx"],
        )
        response = module.build_codex_stop_output(
            decision,
            [],
            stop_hook_active=False,
            review_summary={
                "has_blocker_pages": False,
                "blocker_pages": [],
                "page_results": [
                    {
                        "stepId": "page_02",
                        "status": "missing_sketch",
                    }
                ],
            },
        )

        self.assertEqual(response["decision"], "block")
        self.assertIn("page_02", response["reason"])
        self.assertIn("missing_sketch", response["reason"])

    def test_does_not_block_on_missing_review_surface_outside_focus_steps(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="page 02 左上 PSO 卡和 GPU 重叠了",
            files=["SlideApp/src/remotion/pages/Page02Scene.tsx"],
        )
        response = module.build_codex_stop_output(
            decision,
            [],
            stop_hook_active=False,
            review_summary={
                "has_blocker_pages": False,
                "blocker_pages": [],
                "page_results": [
                    {
                        "stepId": "page_02",
                        "status": "ok",
                        "reviewSource": "formal",
                        "sketchId": "formal-page02",
                        "mechanicalScore": "7.4/10",
                        "verdict": "Clean",
                        "blockerOpen": False,
                        "topFixes": [],
                        "threeLayerReview": {},
                    },
                    {
                        "stepId": "page_03",
                        "status": "missing_sketch",
                    }
                ],
            },
        )

        self.assertTrue(response["continue"])

    def test_allows_continue_when_audits_pass_and_mechanical_review_is_clean(self):
        module = load_workflow_gate_module()

        decision = module.classify_workflow(
            prompt="调整 page_31 的动画 timing",
            files=["Docs/剧本/31-第三十一页-正式动画.md"],
        )
        response = module.build_codex_stop_output(
            decision,
            [],
            stop_hook_active=False,
            review_summary={
                "has_blocker_pages": False,
                "blocker_pages": [],
            },
        )

        self.assertTrue(response["continue"])


if __name__ == "__main__":
    unittest.main()
