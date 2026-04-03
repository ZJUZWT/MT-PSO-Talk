# PSO Animation Demo Design

## Summary

This document defines a small standalone PowerPoint demo that showcases the
"one evolving diagram" presentation style for the PSO talk. The goal is not to
cover the full talk content yet. It is only meant to demonstrate the visual
feel of a concept model that expands slide by slide.

## Goal

- Produce a lightweight animated demo deck the speaker can open locally
- Keep the concept chain focused on the front half of the story:
  - `A -> f(x) -> B`
  - `f(x) = ShaderCode + Pipeline State`
  - `Material -> ShaderCode -> f(x)` for the inline path
  - `Material A/B/C -> ShaderCodeLib -> f(x)` for the shared-code path
- Preserve strong continuity between slides so each next slide feels like an
  evolution of the previous ending state

## Constraints

- The user only wants a demo to inspect the animation effect
- Multi-slide evolution is preferred over packing many click reveals into a
  single crowded slide
- Each slide should still include at least one lightweight animation so the deck
  feels alive when presented
- The demo should be independent from the main talk deck and safe to iterate on

## Storyboard

### Slide 1: Base Transform

- Show a centered three-node chain: `A -> f(x) -> B`
- Present `f(x)` as the single black-box transform
- Use simple click fade-ins for the core chain

### Slide 2: Open the Black Box

- Keep `A` and `B` fixed in place
- Expand the center node into a larger `f(x)` container
- Introduce `ShaderCode` and `Pipeline State` as the two internal ingredients
- Fade in only the newly introduced breakdown and its callout

### Slide 3: Inline Path

- Preserve the same `A`, `f(x)`, and `B` positions
- Add `Material` on the left feeding `ShaderCode`
- Label this branch as `Inline`
- Animate the new material input and label together

### Slide 4: Shared Code Path

- Keep the slide 3 geometry aligned as the starting point
- Expand one `Material` into multiple materials
- Insert `ShaderCodeLib` between the material cluster and `f(x)`
- Label this branch as `Shared Code`
- Animate the new multi-material and library layer

## Visual Direction

- Use a clean widescreen canvas with a pale warm background
- Keep existing structure in muted graphite and gray
- Use one warm accent for the newly introduced inline layer
- Use one cool accent for the shared-code layer
- Favor large readable node labels over dense explanatory text

## Deliverables

- A generated demo deck under `Docs/`
- Slide preview PNGs under a preview folder
- A simple animated preview asset assembled from those slide previews for quick
  local inspection
