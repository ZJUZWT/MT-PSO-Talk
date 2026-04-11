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

If the contract is written as a page script markdown file, each node must also appear in the node review table with:

- Chinese script description when possible
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

If the contract is written as a page script markdown file, each edge must also appear in the edge review table with:

- Chinese script description when possible
- current review viewpoint
- current review score

Treat line text like `cook` or `expand` as segment labels, not fake nodes, unless the user explicitly wants a node.

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
Each page script manages only its own facts and scores. Do not silently carry scores forward from another page.

## Hard Bans

- Do not start drawing before all required nodes and edges are enumerated.
- Do not merge two semantic branches just because they are visually near each other.
- Do not drop line labels like `cook` or `expand`.
- Do not replace exact geometry language with taste words like "balanced enough".
- Do not use a screenshot critique as a substitute for the contract.
- Do not let the rendering step invent missing anchors, bends, or alignments.
- Do not write node or edge review as loose prose when the page script file is supposed to be auditable; use tables.
- Do not omit the current screenshot from the page script markdown once visual iteration has started.
- Do not leave a node or edge out of the review tables just because it currently looks bad.
- Do not skip the `整体布局 Review` score just because node and edge scores already exist.

## Reference

Use [references/contract-template.md](references/contract-template.md) as the default output shape.
