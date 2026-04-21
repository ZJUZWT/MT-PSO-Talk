---
name: write-slide-geometry-contract
description: Use when a slide, diagram, or hand sketch must be translated into explicit node, edge, and layout text before rendering, especially when routing or spatial intent keeps drifting.
---

# Write Slide Geometry Contract

This skill turns a visual idea into a textual assembly contract before any rendering work starts.

Use it to separate:

- semantic actors
- edge meaning
- spatial placement
- visual style
- acceptance checks

The output is not a prose brainstorm. It is a build contract.

When the contract is persisted as a page script markdown file, that file must also work as a review ledger for the current draft.
For this project, store that markdown under `Docs/剧本/`. If it is a sketch mirror rather than the final animation page, mark the filename with `草图镜像-<sketch-id>`.

## When to Use

Use this skill when:

- the user describes a page in words or with a rough hand sketch
- the same page keeps being drawn wrong because semantics and geometry are mixed together
- edge routing quality matters
- line labels carry meaning and cannot be silently converted into nodes
- you need a checklist that can be verified item by item

Do not use this skill for:

- final polish or screenshot critique
- production animation tuning
- pages that are already fully specified in code and only need tiny edits

## Required Output

Always produce these sections in order:

1. `Page goal`
2. `Node inventory`
3. `Edge inventory`
4. `Spatial constraints`
5. `Element styles`
6. `Assembly order`
7. `Acceptance checklist`

If the contract is being stored as a page script markdown artifact for slide iteration, also include:

8. `Current screenshot`
9. `三层 Review 总表`
10. `节点剧本与 Review 表`
11. `边剧本与 Review 表`
12. `过渡动画剧本与时长审查`（当本页存在上一页过渡时）

If the user gave numbered requirements, preserve the numbering in `Acceptance checklist`.

Use the template in [references/contract-template.md](references/contract-template.md).

## Contract Rules

### Node Inventory

Every node must specify:

- `id`
- displayed text
- semantic role
- visual kind
- anchor region on the page
- emphasis level
- required alignments or shared baselines
- whether it has continuity with the previous page
- the required render carrier when continuity exists

If the contract is written as a page script markdown file, each node must also appear in the node review table with:

- Chinese script description when possible
- previous-page continuity note
- current render carrier / shared-element carrier
- continuity review viewpoint
- continuity review score
- current review viewpoint
- current review score

### Edge Inventory

Every edge must specify:

- `id`
- source and target
- meaning
- route grammar
- maximum bend count
- label placement if a segment carries text
- arrow style
- symmetry or sibling relationship when relevant
- whether it has continuity with the previous page
- the required render carrier when continuity exists

If the contract is written as a page script markdown file, each edge must also appear in the edge review table with:

- Chinese script description when possible
- previous-page continuity note
- current render carrier / shared-element carrier
- continuity review viewpoint
- continuity review score
- current review viewpoint
- current review score

Treat line text like `cook` or `expand` as segment labels, not fake nodes, unless the user explicitly wants a node.

### Continuity Semantics Rule

If a node or edge carries continuous semantics from the previous page, the contract must say so explicitly.

That continuity record must include:

- the previous-page counterpart
- whether the current page must reuse the same render element / shared element carrier
- which properties may change during the transition, such as position, scale, label, or emphasis

If the semantics are continuous, the default rule is:

- use the same render element carrier
- do not replace it with a visually similar duplicate that only fades in or out

Only break the same-element rule when the contract explicitly documents why continuity is intentionally broken.

### Three-Layer Review Rule

When the contract is written as a page script markdown file after visual iteration has started, the review ledger must be split into 3 layers:

1. `整体布局 Review`
   - page-level composition, stage occupation, center of mass, and empty-space quality
   - mandatory numeric score
2. `模块空间结构 Review`
   - module/band/lane grouping, inter-module spacing, and route grouping quality
   - mandatory numeric score
3. `单节点与单边 Review`
   - auditable node and edge tables with per-row review comments and scores

Do not collapse these three layers into one loose paragraph.

### Transition Timeline Rule

When the page has a previous-step transition, the page script markdown must include a transition section with:

- `from_step -> to_step`
- frame window and total duration
- duration verdict (`in_range` / `too_short` / `too_long`)
- retiming decision and frame delta when adjustment is needed
- checkpoint table with time/frame and node+edge actions at each checkpoint

Default extraction command:

- `python3 scripts/slide-geometry-harness/probe_transition_timeline.py --from-step <prev_step> --to-step <current_step> --workload-json <workload-json> --emit-markdown`

Use optional `--base-seconds` and `--insert-seconds` to verify "inserted pre-animation must not compress original base duration".

### Spatial Constraints

Write explicit relative geometry such as:

- lower-left / lower-right / upper band
- same horizontal line
- centered between siblings
- exits from lower-right corner
- 45-degree launch then horizontal continuation

Do not use vague placement like "somewhere on the left".

### Element Styles

Specify:

- node box style
- point style if junction points exist
- edge angle family such as `0/45/90 only`
- label style
- any hard visual bans

### Collision Contract Rule

The contract must explicitly declare collision policy before rendering:

- `node-node overlap`: default forbidden; only allowed when declared as container membership
- `edge-edge crossing`: default forbidden; only allowed when crossing point is a declared junction node
- `edge penetration`: default forbidden; edges may not pass through unrelated node/pill/label interiors
- `line-on-line co-lane overlay`: default forbidden; only short declared split/merge handoff segments are allowed

If any exception is required, list it by `id`, with reason and expected visual carrier.

### Assembly Order

Describe the order to place and connect elements so rendering does not improvise topology.

### Page Script Markdown Rule

When the contract lives in a page script markdown file, treat it as the single page-local source of truth for:

- the current screenshot
- the three-layer review summary
- the node table
- the edge table
- the latest review comments

The screenshot, the three-layer review summary, and both tables must be updated together. Do not let the page image drift away from the written review.
When transition semantics exist, update the transition timing section in the same loop as screenshot and review tables.
Each page script manages only its own facts and scores. Do not silently carry scores forward from another page.

This rule applies to both:

- sketch mirror pages
- formal animation pages

Formal pages are not exempt from screenshot, three-layer review, node tables, edge tables, or continuity review.

## Hard Bans

- Do not start drawing before all required nodes and edges are enumerated.
- Do not merge two semantic branches just because they are visually near each other.
- Do not drop line labels like `cook` or `expand`.
- Do not replace exact geometry language with taste words like "balanced enough".
- Do not use a screenshot critique as a substitute for the contract.
- Do not let the rendering step invent missing anchors, bends, or alignments.
- Do not permit node-node overlap unless the contract explicitly marks it as container membership.
- Do not permit edge-edge crossing unless the contract explicitly declares a junction node at that position.
- Do not permit edge penetration through unrelated node/pill/label interiors.
- Do not permit long line-on-line co-lane overlays unless explicitly declared as split/merge handoff.
- Do not write node or edge review as loose prose when the page script file is supposed to be auditable; use tables.
- Do not omit the current screenshot from the page script markdown once visual iteration has started.
- Do not leave a node or edge out of the review tables just because it currently looks bad.
- Do not skip the `整体布局 Review` score just because node and edge scores already exist.
- Do not score only sketch mirror pages while leaving formal animation pages on an older template.
- Do not mark a node or edge as semantically continuous and then re-create it as an unrelated duplicate render carrier without documenting that break.
- Do not finalize a page with previous-page transition semantics while omitting the transition timing section.
- Do not compress original base animation duration when adding inserted pre/mid animation unless user explicitly asks for fixed total duration.

## Reference

Use [references/contract-template.md](references/contract-template.md) as the default output shape.
For all harness formulas and thresholds, use [../../higher-level/slide-geometry-harness/references/harness-formula-registry.md](../../higher-level/slide-geometry-harness/references/harness-formula-registry.md) as canonical.
