# PSO Storyboard API Expansion Design

## Summary

This redesign turns the current diagram demo into a more lecture-ready PSO
teaching sequence. The new flow inserts OpenGL and Vulkan explanation beats
between the abstract `A -> f(x) -> B` model and the existing structural
breakdown pages. The right side remains a single animation canvas, while the
left side evolves from simple notes into a lecture panel that can show speaker
copy plus code snippets with semantic color mapping.

## Audience

- Game client developers in a technical sharing session
- Many viewers are not render specialists
- The talk should explain PSO from first principles, then anchor it in real API
  mental models before entering UE-oriented detail

## Goal

- Preserve the clear top-down teaching arc the speaker wants
- Use the first pages to establish an abstract computation model
- Use OpenGL and Vulkan as concrete API landmarks before entering deeper PSO
  structure
- Keep every page visually focused by making only the current teaching target
  orange
- Fix the current diagram issues:
  - missing arrows in the current inline-material page
  - accidental orange connector in the middle path
  - awkward `ShaderCodeLib` aggregation geometry
  - extra card feeling on the right side

## Revised Story Sequence

### Step 1: Abstract Transform

- Title: `Render Input -> f(x) -> Pixel Output`
- Purpose: establish the irreducible model of "input goes through a function to
  produce visible output"
- Visual rule: only `f(x)` is orange
- Left panel: short speaker copy, no large code block yet

### Step 2: OpenGL State Machine

- Title: `Render Input -> OpenGL State Machine -> Pixel Output`
- Purpose: explain that classic OpenGL does not expose a single PSO object and
  instead requires manually setting shader and fixed-function state
- Visual rule: the OpenGL state cluster is the current orange focus
- Right side: avoid one giant capsule. Use several related state modules or a
  small grouped frame to imply fragmented setup
- Left panel: code snippet with at least two semantic highlight groups:
  - shader-related calls in one accent color
  - pipeline-state calls in a second accent color

### Step 3: Vulkan PSO

- Title: `Render Input -> Vulkan PSO -> Pixel Output`
- Purpose: explain that Vulkan lets the app describe the pipeline as a unified
  object and submit that structure more intentionally
- Visual rule: the unified PSO object is orange
- Right side: one stable object or capsule that reads as "gathered state"
- Left panel: Vulkan pipeline creation code, with the PSO construction path
  highlighted in orange

### Step 4: Open PSO

- Title: `Open PSO`
- Purpose: transition from API-level intuition into PSO internals by opening
  the unified object into `ShaderCode` and `Pipeline State`
- Visual rule: only the active teaching target is orange, default connectors are
  neutral gray
- Right side: current `Open f(x)` page evolves into a PSO expansion page
- Left panel: speaker notes or compact pseudo-structure rather than long code

### Step 5: Inline Material

- Title: `Inline Material`
- Purpose: explain how material-driven specialization flows into shader code
- Fixes required:
  - all three material paths must end in explicit arrows
  - the center connector must not stay orange unless it is the active focus
  - only the current focal path or target should be orange
- Right side: keep the engineered orthogonal layout, but make the direction
  unmistakable
- Left panel: UE-oriented notes or a short pseudo-code excerpt

### Step 6: Shared ShaderCodeLib

- Title: `Shared ShaderCodeLib`
- Purpose: show structural compression from multiple materials into reusable
  shared shader code
- Fixes required:
  - the three material sources can stack like offset cards from lower-left to
    upper-right when overlap would otherwise waste space
  - they should visually converge into one neutral line that points to
    `ShaderCodeLib`
  - `ShaderCodeLib` and its main outgoing link become the orange focus
- Right side: a cleaner, more compact aggregate structure than the current
  version

## Visual Language

### Focus Color

- Orange is reserved for the current concept being taught on that page
- Non-focus wires should return to the neutral dark line color
- This rule applies to both diagram elements and matching code highlights

### Connectors

- Every semantic path ends with an arrow
- Orthogonal routing remains the default
- No decorative or ambiguous orange middle wires
- If a line is not the current concept, it should not demand attention

### Layered Card Motif

- Use a stacked-card motif when concepts overlap but should not consume too much
  screen space
- Best use cases:
  - OpenGL's fragmented state groups
  - material sources feeding shared code
- Stack direction: lower-left toward upper-right
- The stack should imply multiplicity, not random depth

### Right Canvas

- The right side should feel like one single usable stage
- The workbench host should not introduce extra visible nested cards
- The Remotion composition can still draw its own main panel, but it should
  occupy most of the available stage

## Left Lecture Panel

The left side becomes a lecture panel rather than a pure note card.

It should support:

- page title
- manuscript-style framing paragraph
- speaker notes or narration copy
- optional code block
- optional legend that explains color mapping between code and diagram

The code block should feel like slide content, not IDE chrome. It should be
readable at presentation distance and only emphasize the lines relevant to the
current step.

The overall feeling should be "speaker manuscript on the left, animated concept
on the right," not "control dashboard plus preview."

## Top Controls

The three top controls remain useful for iteration, but they should stop
competing with the content.

- Support a collapsed state for the top control area
- Default to collapsed so the slide content gets more vertical space
- Keep the collapsed state visually light and easy to reopen
- When collapsed, the workbench should still preserve the current selected step
  and variant without visual noise

## Bottom Step Navigation

The bottom step navigation should evolve from static equal-width pills into a
centered storyboard rail.

- In the collapsed state, non-active steps can read as circles or compact nodes
- The current step expands into a wider capsule that includes a short label or
  brief summary
- The active step should sit near the center of the rail
- When the user navigates, the rail should scroll or shift so the active step
  recenters
- This rail should feel like presentation navigation, not app pagination

This structure is especially important once the sequence grows from 4 steps to 6
steps.

## Compatibility Strategy

- Keep the current workbench shell, keyboard navigation, and progress bubbles
- Expand the storyboard from 4 steps to 6 steps
- Preserve the overall mental transition from abstract model to detailed PSO
  breakdown
- Retain the existing late-stage material/shared-code pages, but rename and
  restage them to fit the new PSO-first narrative
- Remotion remains the only active rendering runtime for this loop

## Validation

- Chromium and WebKit should render the top controls and main stage consistently
- The left lecture panel must support code and text without collapsing the right
  canvas
- The new OpenGL and Vulkan pages must read correctly even when paused as still
  slides
- `Inline Material` must show explicit arrowheads
- `Shared ShaderCodeLib` must use cleaner convergence and highlight only the
  final shared focus
