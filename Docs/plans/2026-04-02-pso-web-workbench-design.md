# PSO Web Workbench Design

## Summary

This document defines a pivot away from offline video export toward a browser
workbench for rapid iteration. The new deliverable is a slide-friendly web app
that keeps one stable presentation shell while letting the speaker switch
between different animation libraries and layout variants of the same PSO
storyboard.

The workbench is not a generic animation editor. It is a focused comparison and
presentation surface for one technical narrative: starting from `A -> f(x) ->
B`, expanding `f(x)` into `ShaderCode + Pipeline State`, then introducing the
material path and shared shader-code path. The host owns the narrative and the
progress UI; each library owns only how that narrative is rendered.

## Goal

- Replace slow video rerender loops with a browser-first preview workflow
- Support switching between multiple library implementations
- Support switching between multiple layout or motion variants inside each
  library
- Keep the narrative, text, and step progression stable across implementations
- Provide a slide-like progress indicator and readable notes area
- Preserve the existing visual direction: near-white background, orthogonal
  lines, controlled motion, and a technical rather than flashy tone

## Non-Goals

- No full timeline editor in phase one
- No arbitrary node dragging or freeform scene authoring in phase one
- No automatic PPT export in phase one
- No attempt to make `Manim` fully interactive in the browser in phase one

## Audience And Context

- Primary audience: game client developers in a technical sharing session
- Typical usage: the workbench runs on a laptop during content iteration, and
  its stage output informs later PPT pages and speaker notes
- Emotional goal: help the audience form a stable mental model instead of being
  impressed by animation for its own sake

## Success Criteria

- A speaker can change `Library`, `Variant`, and `Step` without leaving the host
  page
- The bottom progress bubbles always reflect the same master storyboard
- The left-side notes and captions remain readable while the right-side stage
  stays crisp on high-DPI displays
- `Remotion` and `Motion Canvas` are both live-switchable inside the host in
  phase one
- `Manim` is available as a comparison surface, even if initially backed by
  prerendered assets instead of a live runtime

## System Architecture

The system is split into four layers:

1. **Host shell**
   - A new browser app under `Docs/prototypes/workbench`
   - Owns page layout, controls, notes, progress bubbles, and state persistence
2. **Shared storyboard contract**
   - A shared data module under `Docs/prototypes/shared`
   - Defines the canonical steps, captions, notes, variants, and capability
     metadata
3. **Library surfaces**
   - Existing packages under `Docs/prototypes/remotion-demo`,
     `Docs/prototypes/motion-canvas-demo`, and `Docs/prototypes/manim-demo`
   - Each package exposes an embed-friendly surface that can respond to host
     state changes
4. **Adapter bridge**
   - Host-side adapters that normalize different rendering surfaces behind one
     contract
   - `Remotion` and `Motion Canvas` use iframe plus `postMessage` bridges in
     phase one
   - `Manim` uses a reference-media adapter in phase one

This keeps the host experience unified without forcing all libraries into one
runtime or bundler.

## Page Structure

The host shell uses one stable layout:

- Top control bar
  - `Library` switcher
  - `Variant` switcher
  - `Aspect` switcher
  - `Step` switcher
  - optional `Play step` and `Reset` actions
- Main content area
  - left: notes panel with current step title, caption, and speaker note
  - right: stage card for the current library surface
- Bottom progress rail
  - semi-transparent bubble markers for each master storyboard step
  - completed, current, and upcoming states use distinct contrast levels

The host shell must reserve a text-safe area and avoid layout shifts when the
active library changes.

## State Model

The host keeps one canonical view state:

- `libraryId`
- `variantId`
- `stepId`
- `aspectRatio`
- `playMode`

State transition rules:

- Switching libraries attempts to preserve the current `variantId` and `stepId`
- If the target library does not support that variant, the host falls back to
  the library default variant
- If the target library cannot scrub to the current step, the host falls back
  to the nearest supported step
- The bottom progress UI always uses the canonical master step list, not the
  child runtime's internal timeline

The host may persist the last selection in the URL query string so links can be
shared inside the team.

## Storyboard Model

The storyboard is shared data, not embedded directly in each animation file.

Each storyboard contains:

- `storyId`
- `title`
- `steps`
- `defaultVariantByLibrary`
- `speakerNotes`
- `layoutHints`

Each step contains:

- `id`
- `label`
- `caption`
- `notes`
- `focusTarget`
- `timingHint`
- `librarySupport`

This lets the same conceptual sequence drive all libraries and the host-side
notes panel.

## Adapter Contract

Each host adapter implements a normalized contract:

- `id`
- `label`
- `mount(container, initialState)`
- `unmount()`
- `setVariant(variantId)`
- `setStep(stepId)`
- `playTransition(fromStepId, toStepId)`
- `getCapabilities()`
- `getSupportedVariants()`

Bridge messaging in phase one:

- parent to child
  - `pso-workbench:set-state`
  - `pso-workbench:play-transition`
  - `pso-workbench:request-capabilities`
- child to parent
  - `pso-workbench:ready`
  - `pso-workbench:capabilities`
  - `pso-workbench:error`

This contract allows different libraries to remain internally different while
still behaving consistently from the host's perspective.

## Library-Specific Strategy

### Remotion

- Keep the existing package at `Docs/prototypes/remotion-demo`
- Add an embed mode that can receive `variant` and `step` updates
- Prefer rendering key paused states crisply and using short transitions between
  them
- This implementation is expected to be the primary library for iteration

### Motion Canvas

- Keep the existing package at `Docs/prototypes/motion-canvas-demo`
- Add a bridge layer that exposes deterministic step entry points
- Preserve the strong bus-first structure and orthogonal line routing
- Motion should use controlled easing only, with no bounce or elastic response

### Manim

- Keep the existing package at `Docs/prototypes/manim-demo`
- Phase one uses prerendered poster, frames, or short video clips mapped to the
  canonical steps
- The host should present it honestly as a comparison mode rather than pretend
  it is equally interactive

## Visual And Motion Rules

- Near-white or white slide-ready background
- One restrained accent color for emphasis
- Rounded boxes and orthogonal connectors only
- Prefer moving nodes to preserve clean line routing instead of bending lines
  awkwardly
- Emphasis comes from scale, opacity, and structural prominence before color
- No bounce, spring, or elastic motion
- Progress bubbles should feel like PPT navigation, not a video scrubber

## High-DPI And Text Clarity

- The stage card should target at least 1280x720 internally in phase one
- Host typography must remain vector crisp and readable on Retina-class screens
- The notes panel should allow quick comparison of caption wording without
  rerendering video assets

## Error Handling

- If a child surface is unavailable, the host shows a clear offline state inside
  the stage card rather than a blank frame
- If a variant is unsupported, the host falls back and shows a small status
  hint
- If a child runtime does not answer the capabilities handshake, the host keeps
  controls visible but disables unsupported actions

## Testing Strategy

- Unit tests for shared storyboard data and library registry behavior
- Component tests for the host control bar, notes panel, and progress bubbles
- Adapter tests for fallback behavior when variants or steps are unsupported
- Build verification for `workbench`, `remotion-demo`, and `motion-canvas-demo`
- Manual smoke check that each library can be selected and responds to step
  changes

## Phase One Deliverables

- New host app under `Docs/prototypes/workbench`
- Shared storyboard contract under `Docs/prototypes/shared`
- Unified bottom progress bubble UI
- Unified notes panel and step caption area
- Working `Remotion` bridge
- Working `Motion Canvas` bridge
- `Manim` reference adapter with canonical step mapping
- One-command local startup for the workbench stack

## Deferred Work

- Full timeline editing
- Exporting back into PPT assets directly from the host
- Arbitrary node authoring tools
- Remote collaboration or synchronized presenter mode
