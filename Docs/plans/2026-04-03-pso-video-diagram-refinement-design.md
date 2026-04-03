# PSO Video Diagram Refinement Design

## Summary

This refinement tightens the right-side animation stage so it reads more like a
finished technical diagram inside a slide deck. The emphasis moves away from
"there is motion" and toward "the space is engineered well." The stage should
occupy more of the available right panel, arrows should feel proportional at all
scales, and the inline/shared-code beats should use cleaner orthogonal bus
layouts.

## Goal

- Make the animation card fill the right-side presentation area more confidently
- Reduce arrowhead size so wires remain readable when the diagram is compact
- Keep line weight, arrowhead scale, and box scale visually coherent across all
  four beats
- Rebuild `Step 3` so materials flow upward into `ShaderCode` through a cleaner
  lower bus
- Rebuild `Step 4` so `ShaderCodeLib` connects to `ShaderCode` with a single
  orthogonal path and no leftover dashed or ghosted inline lines

## Spatial Direction

- The right-side stage should feel like the primary artifact on the page
- The animation card should expand closer to the frame edges while preserving
  enough breathing room for rounded corners and stage chips
- The internal diagram should use the full available panel width instead of
  sitting as a smaller centered island
- The viewer should be able to understand the route of every line without
  tracing around accidental detours

## Arrow System

- Arrowheads should be smaller and sharper than the current version
- Horizontal arrows should stop slightly before boxes so the triangle remains
  readable
- When a path scales down, arrow size and spacing should scale with it so lines
  do not visually disappear behind oversized heads
- Vertical-to-horizontal joints should remain square and orthogonal

## Story Refinements

### Step 1: Base Transform

- Keep `A -> f(x) -> B` simple
- Let the larger right-side stage do the work rather than adding more ornament

### Step 2: Open `f(x)`

- Let the expanded `f(x)` occupy more of the stage width
- Keep `A` and `B` as readable context anchors with proportional arrows

### Step 3: Inline Material

- Materials remain on the lower-left side of the expanded frame
- A horizontal collection bus sits below `ShaderCode`
- Each material routes into that lower bus with short orthogonal joins
- A single short vertical connection lifts from the bus into `ShaderCode`
- This beat should feel like feeding data upward into compiled code, not
  side-loading it from the far left

### Step 4: Shared Code

- The inline lines disappear completely instead of lingering as faded traces
- `ShaderCodeLib` becomes the collection point under `ShaderCode`
- Materials connect into the library via compact orthogonal stubs
- One clean orthogonal accent line runs from `ShaderCodeLib` to the underside of
  `ShaderCode`
- The final layout should emphasize the reusable shared path, not the old inline
  path

## Implementation Notes

- Apply the same geometric rules to both Remotion and Motion Canvas so the
  workbench comparison stays valid
- Favor a small shared set of constants for panel size, arrow scale, and bus
  coordinates
- Keep motion restrained: no bounce, no elastic easing, no decorative path
  drawing beyond the structural reveal

## Validation

- Browser smoke should show larger right-side stage occupancy
- Mid- and end-frame screenshots should clearly preserve line readability
- `Step 3` should visibly route from below `ShaderCode`
- `Step 4` should show no dashed or faded inline leftovers
