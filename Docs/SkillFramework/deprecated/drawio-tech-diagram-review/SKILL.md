---
name: drawio-tech-diagram-review
description: Use when editing or reviewing draw.io diagrams, remotion slide graphics, storyboard visuals, or other technical narrative diagrams where routing, spacing, symmetry, and cross-page evolution must remain consistent.
---

> Deprecated: this legacy skill is archived for reference only and is no longer part of the live SkillFramework workflow for this project.

# Draw.io Tech Diagram Review

Use this skill for draw.io diagrams, remotion slide graphics, and other visuals that explain a technical story, not just isolated boxes and arrows. The goal is to stop "locally fixed, globally broken" edits by forcing narrative checks before layout changes and a rendered second review after edits.

This skill treats `draw.io` as a structural understanding draft. It is not a requirement that the draft already look like the final Remotion/Web scene. The draft exists to prove that the diagram logic, spatial hierarchy, and page-to-page evolution are understood correctly.

However, a draft still needs to clear a minimum visual sanity bar before it is shown as a proposed page direction. "Structural draft" is not a free pass for obviously ugly balance, misleading emphasis, or cramped receiver planes.

## Read First

Before moving any node or reshaping any rendered diagram, read:

- The current source artifact such as `.drawio`, remotion component, or SVG generator
- The current page's script or markdown
- The current page's `Draw.io` rationale section if it exists
- The previous and next page docs when the diagram is part of a sequence
- Any project rubric such as `Docs/线条排版评估Benchmark.md`

If the project has sidecar previews, inspect the rendered preview before and after changes. Do not trust XML coordinates alone.

If the page doc does not yet explain the draw.io design logic, write that rationale before claiming the page is understood.

If you are editing a multi-page sequence, define the sequence bands before touching coordinates:

- the stable runtime spine band
- the upper-source band
- the inserted-layer band
- the page-safe top and bottom margins
- whether a temporarily hidden runtime band still needs to stay visually reserved so later pages can reintroduce it without a jump
- whether a late-page crowding problem should be solved by a whole-sequence shift instead of a local squeeze

Title / cover pages that do not participate in the technical spatial evolution may skip `draw.io` entirely and go straight to Web / Remotion composition.

## Step 0: Clear Physical Blockers First

Before discussing macro composition or narrative polish, run a blunt rendered-image scan for stop-ship errors:

- box-box overlap
- text clipping or cropped labels
- text that is technically visible but too small to read in a classroom / PPT context
- line running through a node body by mistake
- nodes touching with no intentional optical gap
- elements pushed outside the frame or uncomfortably hugging the frame edge
- accidental occlusion caused by a local move

If any blocker exists, fix it first. Do not praise the macro structure while a basic physical error is still on screen.

## Step 0.25: Check PPT Readability Floors

For slide decks that will be projected, do not rely on "it technically fits." Validate rendered font sizes against a readability floor and enlarge the receiver plane or card before shrinking text.

Use these default floors for a 1280x720 stage unless the project already established a stricter rule:

- primary node / receiver titles: `>= 20`
- secondary node titles and important chip labels: `>= 17`
- pills, table values, code tags, and formula rows: `>= 15`
- explanatory copy or routing labels: `>= 14.5`, and only when they are not the page's main teaching point

If the page is intended for PPT or classroom use, add or update automated checks for the critical text on that page. Do not accept "looks okay on my monitor" as evidence.

## Step 0.5: Reject Misleading Drafts Before They Reach The User

Even when the draft is only meant to validate structure, do not show it as a candidate page direction if any of these are true:

- the visual weight is obviously shoved to one side without intentional counter-balance
- the receiver plane is too small to plausibly carry the page's concept
- the highlight color is fighting with 2 or more other emphasis systems
- the primary line is not readable within 2 seconds
- the draft would make a reasonable viewer think the final page direction is cramped, chaotic, or incoherent

If the draft is structurally useful but visually misleading, either:

1. revise it once before showing it, or
2. label it explicitly as `contract-only` and state that it is not yet a composition candidate

Do not silently present a contract-only draft as if it were already a directional layout proposal.

## Step 1: Define The Page Contract

Answer these questions in your own notes before editing:

1. What stable skeleton is inherited from the previous page?
2. What is the new idea this page must introduce?
3. What idea is reserved for the next page and therefore must not be implied here?
4. Which node is the receiver plane (`GPU`, `PSO`, etc.)?
5. Which edge is the primary line and deserves the cleanest channel?
6. Which edges are inherited skeleton, and which edges are the current-page emphasis?
7. Which later-page objects must stay off-stage here so the current page does not imply them too early?

If you cannot answer these, do not edit yet. Read the neighboring docs again.

## Step 1.5: Externalize The Design Rationale

Before or alongside the actual draw.io edit, the page doc must explain:

1. which previous-page skeleton is being preserved
2. which nodes are added, moved, deleted, hidden, or intentionally kept stable
3. which node is the receiver plane and why it deserves its size/position
4. which lines must stay zero-bend and which must bend, plus why
5. why each important edge enters from that side of the source and target node
6. what density, symmetry, layering, or weaving choice the page is using
7. what the draw.io draft is intentionally *not* trying to solve because that belongs to the final Web/Remotion pass
8. which plausible alternative layouts or routings were rejected, and why they lose for this page contract
9. which previous-page nodes are intentionally taken off-stage here, and why removing them is clearer than keeping them
10. which later-page objects are intentionally kept off-stage here, and why omission is clearer than a placeholder
11. if this page forced a global band shift, which neighboring pages were re-checked or moved with it

Do not leave these reasons implicit in your head. If the reasoning is not written down, the next edit will drift.

Prefer a scan-friendly table:

| Subject | Type | Rationale |
| --- | --- | --- |

- `Subject`: the node, edge, or global composition being explained
- `Type`: `Global`, `Node`, or `Edge`
- `Rationale`: why it was added, moved, removed, routed, bent, or kept stable

Also externalize two sequence-wide rules when they matter:

- which page owns the highlight color, and which large receiver planes must stay neutral even if they are visually dominant
- when a page hides lower runtime objects, how the remaining upper band is still anchored to the previous page instead of being freshly re-centered or left-stuck

## Step 2: Use Spatial Properties, Not Literal Instructions

Do not reduce feedback to "move this left 20px" or "贴着". Translate comments into spatial intent:

- `primary channel`: the one line that must stay continuous and unobstructed
- `receiver plane`: the core node treated as a receiving surface, not a random box
- `spatial weaving`: whether left and right groups should interleave through height layering instead of being shoved apart
- `density gradient`: which region should be tighter and which should breathe
- `stepped hierarchy`: whether order should be shown through vertical offsets
- `optical breathing`: small intentional gaps instead of literal touching
- `inserted layer`: a new object such as `PSO` that sits between an inherited upstream band and an existing receiver plane, rather than stealing ownership of an older stable axis
- `descriptor vs payload`: some nodes look input-like but are not runtime data payloads; a descriptor such as `VertexFactory` must not silently collapse into `Vertex Data`
- `reserved band`: even when a page hides a receiver like `GPU`, the absent runtime zone may still need to remain spatially implied so the next page can restore it smoothly
- `off-stage future`: if a later object is not part of the current page contract, omit it instead of inventing a placeholder for it

When local fixes and macro composition conflict, macro composition wins.

Translate repeated feedback into these checks as well:

- `single focus color`: only one page-level focus cluster may use the highlight color; stable receiver planes default to neutral unless the receiver itself is the new concept
- `upper-band anchoring`: if a page hides runtime objects, keep the surviving upper chain on the inherited upper band instead of letting it drift into arbitrary left alignment
- `chrome theft`: if the rendered stage feels too low or too cramped at the top, verify title wrapping, caption height, and shell padding before you overfit the diagram coordinates
- `canvas symmetry`: compare left and right safe margins of the visible composition, not just local gaps between neighboring nodes

## Step 3: Edit In This Priority Order

1. Preserve the previous page's recognizable skeleton.
2. Protect the contrast with the next page's concept.
3. Make the receiver plane wide enough before adding routing complexity.
4. Center the main composition around the receiver plane, not around the widest stray node.
5. Give the primary line the fewest bends possible. If zero bends works, prefer that.
6. Keep routing language consistent without inventing new semantics.
7. Use emphasis color for this page's action, not for every causal relationship.
8. Keep optical gaps between node-node, node-line, and line-line relationships.
9. If you move a real receiver plane that earlier pages intentionally hide, re-open those earlier pages and confirm they still omit it cleanly instead of implying it early.
10. If a later page needs more headroom, consider shifting the whole sequence band before compressing a single page into a special case.
11. If a simplified page keeps only the upper chain, re-check that its remaining chain still inherits the previous page's upper-band anchor and canvas balance instead of floating as a brand-new isolated strip.

Subordinate upstream nodes should be tucked into the main composition, not allowed to drag the whole page off balance.

## Hard Bans

- Do not accept any rendered frame that still has overlap, clipping, collision, or obvious physical breakage.
- Do not use a shared bus or convergence shape if the next page is supposed to reveal convergence for the first time.
- Do not interpret "贴着" as zero gap or collision.
- Do not fix one overlap by creating a worse whole-page imbalance.
- Do not solve spacing problems by adding extra bends first; move groups and widen the receiver plane first.
- Do not claim a newly inserted layer "inherits" a stable axis that actually belongs to an older skeleton line.
- Do not force an inherited stable input line to detour around a new inserted layer if the new layer could yield instead.
- Do not invent dashed future placeholders or ghost targets to patch continuity; if the object is not part of the page contract, keep it off-stage.
- Do not solve late-page crowding by inventing a one-off vertical system for that page while leaving adjacent pages on incompatible bands.
- Do not collapse a descriptor node into a runtime payload node just because both feel like "inputs".
- Do not treat draw.io like a finished product scene and then optimize for stage polish over structural clarity.
- Do not add decorative framing, fake UI chrome, or Remotion-style finish to a draft whose purpose is only to verify understanding.
- Do not declare success from coordinates alone. Always inspect a rendered preview.
- Do not let a large neutral receiver plane keep the highlight color on pages where another object is the actual concept focus.
- Do not assume a diagram is off-center because of node coordinates alone; inspect the surrounding stage/title chrome too.

## Mandatory Second Review

After every meaningful edit:

1. Validate the `.drawio` XML.
2. Refresh the rendered preview, frame export, or screenshot if the project uses one.
3. Re-read the page's written `Draw.io` rationale and check that the rendered result still matches it.
4. Run the blocker scan from [references/checklists.md](references/checklists.md).
5. Only after blockers are clear, review macro composition and narrative.
6. Re-answer the remaining acceptance checks from [references/checklists.md](references/checklists.md).

If any answer fails, revise before claiming the page is improved.

Before presenting a rough draft or a finished visual update to the user, also record the mandatory four-dimension self-review:

| 维度 | 关注点 | 分数 |
| --- | --- | --- |
| `空间美感` | left/right balance, margin symmetry, density rhythm, receiver-plane centering | `0-10` |
| `线条美感` | line cleanliness, bend quality, orthogonality, overlap avoidance, entry-side discipline | `0-10` |
| `表意强度` | page contract clarity, focus strength, page-to-page continuity, semantic readability | `0-10` |
| `演讲适配度` | PPT readability, classroom visibility, narration support, glance comprehension | `0-10` |

For each dimension, add one short reason and one next-fix note. If any score is below `7`, explicitly call out the weakness instead of framing the page as already polished. If any score is below `5`, revise once before showing it unless the user asked to inspect the current broken version.

## Browser Review Loop

For code-rendered slides or Web / Remotion diagrams, a browser render review is mandatory before you call the page done:

1. Jump directly to the target page instead of arrowing through the whole deck.
2. Capture a rendered screenshot from the browser. Prefer the stage-only surface when the shell chrome would distract from the actual diagram judgment.
3. Score the rendered frame on four axes:
   - `Blocker`: overlap, clipping, collisions, accidental occlusion
   - `Routing`: line cleanliness, bend quality, entry-side correctness
   - `Balance`: left/right weight, receiver-plane centering, margin symmetry
   - `Focus`: whether the page-level emphasis is visually obvious
   - `Readability`: whether the projected text sizes still meet the PPT floor in the rendered frame
4. Compute or note the score before claiming success.
5. If any blocker exists, or the page still scores below a convincing threshold, revise first and re-run the screenshot loop.

Do not stop at "the XML looks cleaner" or "the JSX coordinates seem right." The rendered browser frame is the acceptance surface.
## Project Hooks

- If `Docs/线条排版评估Benchmark.md` exists, treat it as an acceptance gate, not inspiration.
- If the deck has per-page docs such as `03-*.md`, `04-*.md`, `05-*.md`, derive layout decisions from page-to-page evolution first.
- If the script folder is the source of truth, each page should carry its own `Draw.io` rationale instead of forcing future editors to reverse-engineer intent from coordinates.
- If earlier pages intentionally omit a later real node, re-check that omission whenever the later real page moves so the narrative boundary still makes sense.
- When a project already uses a preview convention like `.codex-preview/*.svg` and `.png`, keep those previews updated as part of the review loop.
- For remotion or code-generated slide graphics, export or inspect the actual rendered frame before calling the layout good.

## Quick Validation Commands

```bash
xmllint --noout path/to/file.drawio
```

On macOS, if a sidecar SVG preview already exists:

```bash
sips -s format png path/to/preview.svg --out path/to/preview.png
```

## Typical Triggers

- "优化一下这个 drawio 排版"
- "帮我调一下 remotion 里这页图的构图"
- "这页和上一页演化关系不对"
- "为什么这条线这么画"
- "感觉宏观结构不好"
- "这个图表达和 docs 一样吗"
- "帮我 review 这几页结构图"
