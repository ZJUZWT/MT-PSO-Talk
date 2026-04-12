# Slide Geometry Harness Design

**Assumption:** The user explicitly approved a first-pass harness focused on single-page slide geometry, browser-runnable sketch output, latest-only screenshot review, and two parallel blind critics. The user also explicitly wants harness code and helper scripts to stay physically separated from general slide code, while implementation continues directly on `main` without branch-only workflow overhead.

## Goal

Create a `SlideApp`-native harness that turns a short page contract into a browser-runnable single-page geometry sketch, captures the latest real render, and uses isolated review agents to judge layout and line quality before more edits.

## Non-Goals

- general-purpose UI design harness
- final visual styling, typography polish, or production animation fidelity
- multi-page continuity review in v1
- letting the builder agent self-grade its own output
- score systems that rely on freeform taste without image evidence

## Problems Being Solved

| Problem | Root Cause | Design Response |
| --- | --- | --- |
| AI keeps missing the user's mental sketch | The user's intent is implicit and expensive to describe in full | Compress intent into a short `page contract` with a fixed field set |
| Text-only or fake sketches drift from the real page | Drafts are not rendered in the same runtime as the final slide system | Run sketches inside the existing `SlideApp` browser shell on the same local port |
| The user still has to manually open every draft | The system produces descriptions instead of review-ready artifacts | Every round must produce a concrete URL and a latest screenshot |
| Builder self-review is contaminated by implementation context | The same agent that edits the page also judges the result | Route review through two independent blind critics |
| Scores are not trustworthy | Freeform judging creates score hallucination | Require a fact layer, a metric layer, and rule-bound scores before verdicts |

## Approved Direction

The first version of the harness is intentionally narrow:

- slide-only, not general UI
- single-page geometry quality only
- browser-runnable sketch output, not prose-only drafts
- simple boxes, lines, and labels are acceptable
- latest-only screenshot output
- no side-by-side comparison rendering
- no second dev server or second port

## Chosen Architecture

### 1. Keep sketch mode inside the existing `SlideApp`

Do not create a second preview app or a second port. Sketch pages should run inside the current app shell and share the same stage, review mode, and capture conventions.

Preferred URL shape:

```text
http://127.0.0.1:4173/?mode=sketch&sketch=<sketch-id>&review=1
```

This keeps the user, the builder, and the critics anchored to the same runtime surface.

### 1.5 Physically separate harness runtime code and helper scripts

Even though the sketch runs inside the same browser app, harness code should not be scattered across unrelated slide modules.

Use a dedicated runtime subtree for harness-specific code:

- `SlideApp/src/harness/slide-geometry/`

Use a dedicated helper-script subtree for harness-specific scripts when scripts are needed:

- `scripts/slide-geometry-harness/`

This keeps the harness easy to inspect, archive, or replace later without pretending it is a separate app.

### 2. Split semantic intent from geometric draft state

The harness should distinguish:

- the semantic page contract
- the current geometric sketch candidate

The contract explains what the page is trying to do. The sketch candidate records where boxes and orthogonal lines currently live.

This separation matters because future rounds should be able to re-layout the page without rewriting the original semantic intent.

### 3. Use a short fixed page contract

The builder should not require a full paragraph or a hand-drawn mockup. The first-pass contract is intentionally short:

| Field | Purpose |
| --- | --- |
| `Page Goal` | What the page must teach or prove |
| `Receiver Plane` | The core visual receiver or dominant box |
| `Primary Line` | The one route that must read first |
| `Keep Stable` | What must remain recognizable in this round |
| `New Change` | What this round introduces or rebalances |
| `Do Not` | Explicit failure modes to avoid |

This is enough to anchor generation without turning the user into a diagram tool.

### 4. Render a contract-only geometry sketch, not a fake finished slide

The first version of the sketch should be allowed to use:

- rectangles
- orthogonal polylines
- simple pills
- minimal labels

It should not try to imitate the full production look. The goal is to test:

- spacing
- balance
- line cleanliness
- primary channel clarity

The sketch must still run in the real browser environment and respect the slide stage size.

### 5. Produce review-ready artifacts every round

Every completed round must emit:

- a concrete sketch URL
- a latest screenshot
- capture provenance
- current facts, metrics, and scores
- the next `Top 3` fixes

The screenshot should be `stage-only` by default. Browser chrome should be treated as fallback evidence, not the preferred review surface.

### 6. Keep capture and loop control mechanical

Not every harness role should be an agent.

The recommended split is:

- main agent: build or modify the sketch
- capture layer: mechanical tool/script path
- blind art critic: isolated subagent
- blind geometry critic: isolated subagent
- loop control: main agent

Capture and loop control are execution problems, not judgment problems. They should stay deterministic and lightweight.

### 7. Use two parallel blind critics

The harness should run two independent reviewers in parallel:

#### Art Critic

This reviewer focuses on:

- spacing rhythm
- page tightness vs looseness
- mass balance
- hierarchy
- focus clarity

#### Geometry Critic

This reviewer focuses on:

- line crossing
- unnecessary bends
- "can be straight but is not" failures
- entry-side cleanliness
- primary route clarity
- stop-frame explainability

Both critics must be blind:

- they should not see the builder's self-justification
- they should not see each other's output
- they should not edit the sketch directly

Each critic only receives:

- the latest screenshot
- the page contract
- any extracted mechanical metrics needed for scoring

### 8. Forbid freeform scoring

The harness must reject score hallucination by forcing three layers:

#### Fact Layer

The reviewer must first extract plain observations such as:

- overlap present or absent
- crossing present or absent
- primary line bend count
- avoidable bend count
- minimum box-to-box gap
- minimum margin to frame edge
- obvious one-sided mass imbalance
- unreadable or undersized labels

#### Metric Layer

Those facts then collapse into bounded metrics such as:

- `overlap_count`
- `crossing_count`
- `primary_line_bend_count`
- `avoidable_bend_count`
- `min_gap_px`
- `edge_margin_min_px`
- `left_right_mass_delta`
- `readability_floor_violations`

#### Score Layer

Scores are then rule-mapped from metrics rather than guessed.

Example policy:

- if `crossing_count > 0`, the line-crossing score is capped at `4`
- if `overlap_count > 0`, the blocker verdict stays open
- if the primary route has avoidable bends, the straightness score cannot be high

The final language should explain the score, not invent it.

### 9. Stop early, but never silently

The loop should run at most three automatic refinement rounds.

It may stop early only if:

- no overlaps remain
- no line crossings remain
- the primary route reads cleanly
- both critics judge the page above threshold

If the third round still fails, the harness must stop and explicitly report which constraints remain unsolved.

## Directory Target

```text
SlideApp/src/
  harness/
    slide-geometry/
      contracts/
        <sketch-id>.ts
      registry/
        sketchRegistry.ts
      render/
        GeometrySketchScene.tsx
        geometry-sketch-types.ts
      review/
        geometryMetrics.ts
        geometryScorePolicy.ts
        criticPromptTemplates.ts
scripts/
  slide-geometry-harness/
    ...
```

This keeps harness-specific logic separate from production Remotion scene modules while still living inside the same app and on the same port.

## Review Artifact Contract

Every completed round should produce this shape:

| Field | Meaning |
| --- | --- |
| `url` | The exact browser-runnable sketch URL |
| `screenshot_path` | Latest captured image path |
| `capture_mode` | `stage-only` or fallback window capture |
| `facts` | Plain visual observations |
| `metrics` | Mechanical counters or measurements |
| `art_review` | Blind art critic verdict |
| `geometry_review` | Blind geometry critic verdict |
| `top_fixes` | The next 3 highest-leverage adjustments |

## Why This Is Better Than Alternatives

### Rejected: Text-only sketch review

This would keep the current failure mode where the AI and the user imagine different pages.

### Rejected: Builder agent self-evaluation

This would make score inflation and rationalization almost unavoidable.

### Rejected: A second sketch app or second port

That would recreate the draft-vs-real mismatch the harness is supposed to remove.

### Rejected: Multi-page continuity in v1

Continuity matters later, but it is not the primary bottleneck. Single-page geometry quality is the harder unsolved problem right now.

## Success Criteria

1. A short contract can generate a browser-runnable sketch URL inside `SlideApp`.
2. Every round emits a latest screenshot without requiring a manual browser inspection first.
3. The builder agent never self-grades.
4. Two blind critics can independently identify spacing, balance, and routing problems from the same latest screenshot.
5. Scores are explainable from facts and metrics rather than freeform taste.
6. After at most three rounds, the harness either converges or clearly reports what blocked convergence.
