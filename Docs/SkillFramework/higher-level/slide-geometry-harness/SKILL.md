---
name: slide-geometry-harness
description: Use when iterating SlideApp slide layouts and you need a browser-runnable sketch that mirrors a real finished slide, a screenshot from the visible browser result, and blind dual-critic review before more edits.
---

# Slide Geometry Harness

This skill is the slide-specific orchestration layer for `SlideApp` geometry iteration.
It is a higher-level skill, not a disguised `drawio` review loop.

It composes:

- textual geometry contract drafting
- browser-runnable sketch generation
- latest-only screenshot capture
- fact-bound geometry metrics
- transition timing probe and checkpoint script extraction
- two blind critics
- lesson harvesting when durable workflow rules or user preferences emerge

Use this skill for slide geometry work, not for general UI review.

Keep the mechanics physically separated:

- runtime sketch code under `SlideApp/src/harness/slide-geometry/`
- harness scripts under `scripts/slide-geometry-harness/`

If the job is only "capture what Edge shows and judge whether it looks good", use `edge-image-aesthetic-review` instead.

## When to Use

Use this skill when:

- the user has a rough slide idea but not a polished sketch
- the user already has a finished or semi-finished slide and wants the sketch to mirror that page first
- the main problem is single-page layout, spacing, or line quality
- the draft must run in the real `SlideApp` browser shell
- the builder should not be allowed to self-grade

Do not use this skill for:

- final polish or production styling
- general website or product UI critique
- multi-page continuity as the primary problem

## Required Workflow

1. If a finished slide already exists, mirror that slide's real structure into the sketch first. Do not start from an abstract demo surrogate.
2. Before rendering anything, write a full page contract with `write-slide-geometry-contract`.
3. The contract must enumerate every node, every edge, every line label, the angle family, the spatial alignments, and the numbered acceptance checklist.
4. If the page contract is stored as a markdown script file, keep it under `Docs/剧本/`. If it is a sketch mirror, mark the filename with `草图镜像-<sketch-id>`. Formal animation pages and sketch mirrors both use the same review ledger structure. That file must also include the latest screenshot, a three-layer review summary, and auditable node and edge review tables.
5. The three-layer review summary must score:
   - `整体布局 Review`
   - `模块空间结构 Review`
   - `单节点与单边 Review`
6. The node and edge review tables must contain Chinese script descriptions when feasible, followed by continuity review, current render carrier, review viewpoint, and review score columns.
6.5. Before any visual score is accepted, run a continuity audit against the previous page:
   - identify every node with continuous semantics
   - identify every edge with continuous semantics
   - record whether each one must reuse the same render element / shared-element carrier
   - if continuity exists, treat duplicate re-creation as a blocker unless the contract explicitly allows a break
6.6. For every page transition, extract timing facts before final scoring:
   - run `python3 scripts/slide-geometry-harness/probe_transition_timeline.py --from-step <prev_step> --to-step <current_step> --workload-json <workload-json> --emit-markdown`
   - the workload JSON must enumerate node/edge/fade action items so timing is formula-driven instead of ad-hoc
   - store duration verdict, timing-standard verdict, phase timeline table, action timing table, and checkpoint table (time/frame/node action/edge action) in the current page markdown script
6.7. If the timing probe says `too_short` or `too_long`, retime the transition by adjusting frame anchors in `SlideApp/src/remotion/embed.ts`, then update timeline/storyboard tests.
6.8. If a pre-animation is inserted, preserve the original base animation duration; do not compress base motion to make room for the insertion unless the user explicitly requests a fixed total duration.
6.9. Harness formulas are centralized in `harness-formula-registry.md`; timing formulas in `animation-timing-standard.md` are a focused subset and must stay consistent with the registry.
7. Only after the contract is explicit may you build or revise a sketch under `SlideApp/src/harness/slide-geometry/`.
8. Expose the sketch through the existing `SlideApp` port with a concrete URL.
9. Capture the latest rendered result using the SlideApp browser export path first:
   - primary automation path: `browser-api` capture of the real SlideApp page or stage
   - manual reality check: current front browser window capture when you need to compare against what the user literally sees on screen
   - fallback: `surface=stage` headless screenshot only when browser export is unavailable
10. Update the page script markdown screenshot, the three-layer review summary, and both review tables after capture, before critic merge.
10.5. Keep each page script responsible only for its own scores and review facts. Do not auto-propagate numeric judgments from one page into another page's ledger.
10.6. After page edits, rerun transition timing probe and append an animation review verdict for the edited transition.
11. Extract facts and metrics before any score is allowed.
11.5. Line scoring must not stop at raw bend count. The fact layer must also audit, at minimum:
   - whether a line stayed bent when it could have been materially straighter
   - whether exits and arrivals are reasonably centered on their entry side when layout allows it
   - whether the route introduces short hooks, redundant detours, edge-on-edge overlap, or other visually twisted segments
   - whether any non-centered anchor is justified by the surrounding layout rather than being a lazy corner stab
11.6. Treat these routing rules as nuanced penalties, not blind absolutes:
   - if a line can be fully straight with no meaningful layout cost, prefer straight and penalize avoidable bending
   - if a line can stay near the side center with no meaningful layout cost, prefer centered anchors
   - if preserving global composition requires a controlled bend or offset anchor, allow it
   - never allow obviously awkward twists, hook turns, or line lanes piled on top of each other
11.7. Collision and penetration gates are hard-fail checks unless the contract explicitly whitelists them:
   - any node-node overlap area greater than `0` is a blocker unless it is declared container membership
   - any edge-edge crossing is a blocker unless the crossing is a declared junction node
   - any edge that penetrates the interior of an unrelated node/pill/label is a blocker
   - any long edge-on-edge co-lane overlap is a blocker unless it is a declared split/merge handoff
11.8. If any gate in `11.7` fails:
   - cap `单节点与单边 Review` at `4.0`
   - mark the page `not approved` until geometry is corrected
12. Send the latest artifact bundle to two blind critics in parallel:
   - `Art Critic`
   - `Geometry Critic`
13. Critics see the latest screenshot plus the contract. They do not see builder self-justification.
14. Critics must score the three review layers separately before any merged verdict:
   - `整体布局 Review`
   - `模块空间结构 Review`
   - `单节点与单边 Review`
15. Merge both reviews into one next-step verdict.
16. If the user explicitly asks to remember, persist, or write the lesson into skill or memory, invoke `conversation-lesson-harvester` before closing the task.
17. Stop after at most 3 automatic rounds, or earlier if the stop rule passes.

## Capture Rule

For agent automation, the default export path is the browser-native SlideApp capture API.

Use:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope page --url "<slide-url>"`

For stage-only export:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode browser-api --scope stage --url "<slide-url>"`

Use front-window capture when the question is specifically about what the user literally saw in the browser window, including shell chrome or mismatch reports.

Do not fall back to screen-region capture when the browser-native export path is available.

Use front-window or headless only when:

- browser export is unavailable
- browser export failed
- you need a second source to compare with the visible browser shell

Recommended fallback:

- `bash scripts/slide-geometry-harness/capture_review_png.sh --mode headless-stage --url "<slide-url>"`

## Required Output

Every completed round must include:

- `Element contract`
- `Page script markdown`
- `URL`
- `Latest screenshot`
- `Capture provenance`
- `Capture size`
- `Facts`
- `Metrics`
- `Continuity facts`
- `Transition timing report`
- `Timing standard plan`
- `Timing standard verdict`
- `Timing standard phase timeline`
- `Timing standard action table`
- `Transition checkpoint script`
- `Three-layer review summary`
- `Animation review verdict`
- `Art critic verdict`
- `Geometry critic verdict`
- `Top 3 fixes`

## Hard Bans

- Do not let the builder agent score its own sketch.
- Do not let critics read the builder's self-justification.
- Do not review an old screenshot when a newer one exists.
- Do not start from a fake generic diagram when the real slide already exists.
- Do not treat a vague sketch as a substitute for a node/edge contract.
- Do not render before nodes, edges, line labels, and alignments are enumerated.
- Do not keep screenshot, node tables, and edge tables in different sources of truth.
- Do not keep screenshot, three-layer review summary, node tables, and edge tables in different sources of truth.
- Do not write a page script markdown file that has edges reviewed in prose but nodes reviewed in tables; keep the format symmetric and auditable.
- Do not skip the `整体布局 Review` score just because the local routing looks correct.
- Do not keep using screen-region capture when browser-native export already works.
- Do not omit capture bounds or browser-chrome status when reporting provenance.
- Do not emit side-by-side comparison renders by default.
- Do not use freeform scores with no fact layer.
- Do not give a high line score merely because crossings are zero; awkward hooks, redundant detours, line-on-line coverage, and lazy off-center anchors must still be penalized.
- Do not pass review when node-node overlap exists unless that overlap is explicitly whitelisted as container membership.
- Do not pass review when edge-edge crossing exists unless the crossing is an explicitly declared junction.
- Do not pass review when a line penetrates the interior of an unrelated node, pill, or label.
- Do not pass review when long line-on-line overlay exists unless it is an explicitly declared split/merge handoff.
- Do not keep formal animation pages on an older page template while only sketch mirror pages get the new review ledger.
- Do not let a semantically continuous node or edge switch to an unrelated duplicate render carrier without flagging it as a continuity failure.
- Do not finalize a page without a transition timing section in the page markdown when the page has a previous-page transition.
- Do not leave checkpoint node/edge action rows as unresolved TODOs in final output.
- Do not compress original base animation duration when adding pre/mid animation unless the user explicitly asks for fixed total duration.

## Reference

See [references/harness-formula-registry.md](references/harness-formula-registry.md) for the unified formula table, [references/loop-contract.md](references/loop-contract.md) for the control loop, [references/animation-timing-standard.md](references/animation-timing-standard.md) for timing-focused details, and [references/critic-prompts.md](references/critic-prompts.md) for the two blind critic roles.
