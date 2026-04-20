# Formal Page Review Scoring Design

## Goal

Make formal Remotion pages the default scoring target for slide review, while reusing the existing harness formulas, metrics, and score policy instead of inventing a second scoring system.

## Why change

- The current geometry review chain is mechanically strong, but it is wired around `sketch` inputs.
- The project target has already moved to formal pages, so review should score the formal page itself instead of a sketch surrogate.
- The user wants agent self-correction to be driven by computed scores from existing formulas, not by manual review-only heuristics.

## Non-negotiable rules

1. Formal pages must be scoreable.
2. Existing formulas remain the single source of truth.
3. No new manual-only score bands are introduced for formal pages.
4. Hooks may trigger scoring and enforce the loop, but hooks must not contain scoring logic.

## Current constraint

The current review path computes a geometry artifact only when review mode is paired with a geometry sketch:

- `SlideApp/src/App.tsx`
- `SlideApp/src/review/page19PlusReview.ts`

This means the score engine exists, but formal pages are not yet first-class review inputs.

## Design

### 1. Formal page review becomes the default path

Introduce a formal-page review pipeline:

- formal page render -> formal review primitives
- formal review primitives -> existing geometry metrics / score policy
- geometry scores + timing verdicts -> hook feedback for self-correction

Priority order:

1. formal page review artifact
2. legacy sketch review artifact
3. manual-only fallback

### 2. Do not parse SVG as a source of truth

The math engine does not need raw SVG text. It needs structured geometry:

- node boxes
- edge routes
- text containers
- page bounds / spacing facts

So the formal-page path should not reverse-engineer final SVG markup.
Instead, formal page code should expose review primitives directly from the same layout facts already used to render the page.

### 3. Reuse existing score chain

The formal-page pipeline must reuse these existing functions:

- `collectGeometryMetrics(...)`
- `scoreGeometryMetrics(...)`
- `buildGeometryReviewArtifact(...)`
- existing timing probe / timing audit scripts

To do that cleanly, formal review primitives should be adapted into the same geometry input shape expected by the current metric collector, or into a compatible sibling type that is converted into that shape with no formula changes.

### 4. Split structural facts from rendered text facts

Formal review needs two fact layers:

1. Structural facts
- box positions
- route points
- group spacing
- overall margins / density / balance

2. Rendered text facts
- actual font size
- line count
- overflow
- tightest text padding

Structural facts should come from formal page layout definitions.
Rendered text facts should come from browser measurement, similar to the current text probe pattern.

### 5. First version scope

The first formal-page scoring version should cover the highest-value checks already supported by existing formulas:

- node internal text overflow
- node internal padding / text spacing
- node-to-node spacing
- page outer margins
- side clearance / density
- overall stage balance

It should also preserve the existing timing review flow so final stop feedback includes:

- formal page geometry score
- three-layer review scores
- timing verdict
- blockers
- top fixes

### 6. Edge handling in v1

The first version should not attempt perfect semantic inference for every implicit relationship in text-heavy pages.

Instead:

- explicit arrows / routes are treated as formal edges
- obvious reading-flow transitions may be modeled conservatively
- pages with weak edge semantics may still receive strong node / spacing scores

This keeps v1 focused on trustworthy formal-page scoring instead of overreaching into fragile inference.

## Architecture

### New pieces

- formal review primitive types
- formal review primitive extractor
- formal review artifact builder that adapts formal primitives into the existing geometry score chain
- browser text probe integration for formal review nodes

### Existing pieces to reuse

- `SlideApp/src/harness/slide-geometry/review/geometryMetrics.ts`
- `SlideApp/src/harness/slide-geometry/review/geometryScorePolicy.ts`
- `SlideApp/src/harness/slide-geometry/review/geometryReviewArtifact.ts`
- `SlideApp/src/review/geometryReviewSummary.ts`
- `scripts/slide-geometry-harness/probe_transition_timeline.py`
- `scripts/slide-geometry-harness/audit_transition_timings.py`

### Existing entrypoints to update

- `SlideApp/src/App.tsx`
- `SlideApp/src/components/ReviewHud.tsx`
- `SlideApp/src/review/page19PlusReview.ts`
- Codex workflow gate / stop feedback path

## Hook integration

The Codex hook layer should stay thin.
It should:

- trigger the formal-page score calculation
- collect the score payload and timing verdicts
- block completion when thresholds or blockers fail
- send the score payload back to the agent as self-correction context

It should not:

- create new formulas
- duplicate score thresholds in multiple places
- contain page-specific review logic

## Expected output for the agent loop

The self-correction loop only needs a compact machine-usable payload:

- total mechanical score
- `整体布局 Review`
- `模块空间结构 Review`
- `单节点与单边 Review`
- blocker list
- top 3 fixes
- timing verdict summary
- pass / retry decision

## Out of scope for v1

- parsing raw SVG markup as the review source
- inventing a second formal-page-only formula system
- demanding perfect implicit-edge inference for every text page
- replacing the existing timing standard
- building a large markdown reporting system before the score loop works

## Success criteria

1. Review mode can compute a score on formal pages without requiring sketch mode.
2. The score comes from the existing geometry score chain.
3. Stop-phase hook feedback includes formal-page scores plus timing verdicts.
4. Agent self-correction can use those results to continue iterating.
5. Manual-only fallback becomes the exception rather than the default.
