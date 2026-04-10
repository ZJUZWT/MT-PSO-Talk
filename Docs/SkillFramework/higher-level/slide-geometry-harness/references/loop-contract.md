# Loop Contract

## Artifact Bundle

Every loop iteration should produce one latest-only artifact bundle:

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

## Source-of-Truth Rule

If the slide already exists in the real browser shell, mirror that page before simplifying geometry.

If the formal page inherits a semantic lane, table, or field band from an earlier page, the sketch must mirror that inherited structure before any cleanup pass.

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
- both critics rate the page above the target threshold

The loop must stop after 3 rounds even if it does not converge. When that happens, report the unsolved blockers explicitly.

## Merge Rule

- If both critics mark the same blocker, it becomes mandatory next-round work.
- If one critic marks a blocker and the other does not, keep it open unless the fact layer clearly disproves it.
- If both critics suggest the same improvement class, it should rank near the top of `Top 3 fixes`.
- If their recommendations conflict, prefer the interpretation that best matches the page contract.

If both critics independently describe the same issue as "cramped", "too tight", or "not enough breathing room", treat that as a real layout blocker even when there is no overflow.
