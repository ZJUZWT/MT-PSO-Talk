# Loop Contract

## Artifact Bundle

Every loop iteration should produce one latest-only artifact bundle:

- `element_contract`
- `page_script_markdown`
- `url`
- `screenshot_path`
- `capture_mode`
- `capture_url`
- `browser_chrome_included`
- `capture_bounds_css`
- `image_size_px`
- `page_contract`
- `facts`
- `metrics`

The bundle is the only thing sent to critics.

Add one more required field when the page has a previous-page transition:

- `continuity_facts`
- `transition_timing_report`
- `timing_standard_plan`
- `timing_standard_verdict`
- `timing_standard_phase_timeline`
- `timing_standard_action_table`
- `transition_checkpoint_script`
- `animation_review_verdict`

## Contract-First Rule

Before any rendering pass, the builder must write a text contract that fully enumerates:

- nodes
- edges
- line labels
- junction points
- spatial alignments
- element styles
- acceptance checklist

If any of those are missing, the page is still in specification mode and should not be graded as a visual draft.

The default contract writer for this step is `write-slide-geometry-contract`.

All harness formulas (geometry scoring + typography + timing) use one canonical table:

- `references/harness-formula-registry.md`

The contract is not allowed to say only "roughly here" or "similar to the sketch". It must describe anchor regions and routing language explicitly enough that another agent could place the same topology.

When a page-local markdown script file exists, it must be updated in the same loop with:

- the latest screenshot
- a three-layer review summary
- a node review table
- an edge review table
- a transition timing section (when the page has a previous-step transition)
- a timing-standard section (formula version, phase timeline, action timing table)

When transition timing is generated, prefer formula-driven timing:

- provide a `workload-json` that enumerates transition actions (`node_move`, `edge_grow`, `fade_in`, `fade_out`)
- use `probe_transition_timeline.py --workload-json ...` to compute per-action duration and phase schedule
- do not accept "looks fine" timing without computed action metrics

The three-layer review summary must always include:

- `整体布局 Review` with a numeric score
- `模块空间结构 Review` with a numeric score
- `单节点与单边 Review` with a numeric score

Both tables must expose, per row:

- the element id
- a Chinese script description when feasible
- previous-page continuity note
- current render carrier
- continuity review viewpoint
- continuity review score
- the current review viewpoint
- the current review score

## Source-of-Truth Rule

If the slide already exists in the real browser shell, mirror that page before simplifying geometry.

If the formal page inherits a semantic lane, table, or field band from an earlier page, the sketch must mirror that inherited structure before any cleanup pass.

If the formal page and previous page share a continuous semantic actor or route, the contract must say whether the same render carrier is required.

Default rule:

- semantically continuous node -> same render carrier
- semantically continuous edge -> same render carrier

Do not silently downgrade that into "looks similar enough".

Do not replace a meaningful inherited lane with an empty placeholder bar. If the formal page still shows fields, labels, or branch semantics, the sketch must keep them.

For agent automation, prefer the browser-native SlideApp export first because it produces a deterministic local PNG from the actual rendered page DOM.

If both of these exist:

- a browser-api export
- a front-window browser capture

prefer the browser-api export for mechanical review loops and keep the front-window capture as visual dispute evidence.

Use the front-window or headless artifact only as fallback or supplemental evidence.

Every capture report should preserve enough information to explain visual mismatch:

- browser URL
- whether browser chrome is included
- CSS bounds used for capture
- final PNG pixel size

## Coupled Layout Rule

Treat slide readability as one coupled inverse problem:

- font size
- line breaking
- box width and height
- inter-box gutter
- edge anchor positions
- inherited semantic lanes

Do not "fix" any one of these in isolation.

If a label must grow for PPT readability, reflow the layout around it instead of shrinking the requirement back down.

If larger type causes overflow, the next move is to change layout geometry, not to pretend the smaller pre-change layout was acceptable.

Passing `text_overflow_count = 0` is necessary but not sufficient. A page can still fail if text only technically fits while the layout becomes cramped or semantically distorted.

## Formal Mirror Invariants

When a finished formal slide exists, preserve these invariants during sketch mirroring:

- keep the same major semantic actors
- keep inherited bottom or side lanes that still carry meaning
- keep the same branch count when branches encode distinct concepts
- keep distinct fields distinct when the formal page uses them for explanation

For page09 specifically:

- `PSO Cache` is not an empty footer band
- `VS Hash` and `PS Hash` must remain visible as distinct fields
- the two hash references are separate routes into the `ShaderHashTable` path
- removing those fields or merging them into one generic line destroys the stop-frame meaning

## Role Split

- `Builder`: edits the sketch and never self-grades
- `Capture`: creates the latest screenshot mechanically
- `Art Critic`: judges spacing, balance, hierarchy, and density
- `Geometry Critic`: judges crossings, bends, straightness, and primary-line clarity
- `Loop Controller`: merges verdicts and decides whether to continue

Both critics must score the page at 3 nested layers before final verdict:

1. `整体布局 Review`
2. `模块空间结构 Review`
3. `单节点与单边 Review`

## Capture Order

1. Export the page or stage through `capture_review_png.sh --mode browser-api`.
2. Capture the visible front browser window only if you need to compare against what the user literally saw.
3. Use headless `surface=stage` capture only if steps 1-2 are unavailable or failed.

## Stop Rule

The loop stops early only when:

- `overlap_count = 0`
- `crossing_count = 0`
- `text_overflow_count = 0`
- the primary route is clean enough to explain at a glance
- transition duration verdict is not `too_short` / `too_long`
- timing standard verdict is not `too_short` / `too_long`
- both critics rate the page above the target threshold

The loop must stop after 3 rounds even if it does not converge. When that happens, report the unsolved blockers explicitly.

## Merge Rule

- If both critics mark the same blocker, it becomes mandatory next-round work.
- If one critic marks a blocker and the other does not, keep it open unless the fact layer clearly disproves it.
- If both critics suggest the same improvement class, it should rank near the top of `Top 3 fixes`.
- If their recommendations conflict, prefer the interpretation that best matches the page contract.

If both critics independently describe the same issue as "cramped", "too tight", or "not enough breathing room", treat that as a real layout blocker even when there is no overflow.

## Contract Review Rule

Critics must compare the screenshot against the contract, not against the builder's intent summary.

They should explicitly flag:

- missing nodes
- missing or merged branches
- lost line labels
- broken baseline or band constraints
- violations of the declared angle family
- branches whose bend count exceeds the contract

If a page script markdown file exists, critics should also verify:

- the three-layer review summary exists and matches the latest screenshot
- every visible node appears in the node review table
- every visible edge appears in the edge review table
- every semantically continuous node records its continuity score and render carrier
- every semantically continuous edge records its continuity score and render carrier
- the latest screenshot in the markdown matches the latest reviewed artifact
