# PSO Video Demo Design

## Summary

This document defines a replacement for the previous PowerPoint-style animation
demo. The new deliverable is a single-camera video system that presents the PSO
concept chain as one evolving diagram on a single canvas and also exports
page-sized subclips for direct use inside PowerPoint.

The visual goal is closer to a lightweight "math animation" feel than a slide
transition feel: rounded boxes grow, shrink, split, and accept new content.
Nothing should feel like a page flip.

## Goal

- Produce a short video demo that feels direct, visual, and intuitive
- Keep the entire motion on one canvas without slide cuts
- Fit naturally into a white-background slide deck
- Reserve a text-safe area on the left side of the frame
- Show the front-half concept evolution:
  - `A -> f(x) -> B`
  - `f(x)` expands and reveals `ShaderCode + Pipeline State`
  - `Material` enters and attaches to `ShaderCode`
  - multiple `Material` nodes compress into a `ShaderCodeLib`
- Make focus obvious by scaling up the current concept and shrinking secondary
  context

## Motion Principles

- Use rounded rectangles as the primary language
- Use smooth interpolation for position, size, opacity, and scale
- Treat the central `f(x)` node as the stable anchor of the world model
- Keep a stable skeleton for the major node positions so the audience does not
  lose the mental map
- When a concept becomes the focus:
  - enlarge it
  - move it toward center
  - reduce emphasis on everything else
- When a new concept is introduced:
  - derive it from an existing shape
  - slide or grow it into place
  - avoid abrupt appear/disappear cuts
- Do not use elastic or bouncy motion
- Favor crisp, faster pacing suitable for a talk slide rather than a standalone
  cinematic intro
- Keep arrows orthogonal and delay them until the target boxes are stable
- Do not allow keyframe overlap in the paused reading states

## Story Beats

### Beat 1: Base Transform

- Show `A -> f(x) -> B`
- Keep the composition simple and centered

### Beat 2: Open `f(x)`

- The `f(x)` frame grows
- `ShaderCode` and `Pipeline State` emerge inside it
- `A` and `B` shrink slightly to become context

### Beat 3: Inline Material Path

- `ShaderCode` becomes the focus region
- `Material` slides in from the left
- A visual connection forms from `Material` to `ShaderCode`

### Beat 4: Shared Code Path

- The single `Material` duplicates into three smaller material nodes
- A `ShaderCodeLib` box forms between the materials and the core transform
- The final composition settles into a readable overview state

## Layout Direction

- Canvas: 1920x1080
- Left side: intentionally sparse white text-safe area
- Right side: animation card region
- The animation region can use a very subtle elevated card or panel, but the
  full frame must still read as a white slide

## Visual Direction

- Background: near-white
- Base shapes: charcoal + soft gray
- Accent system: one emphasis color only
- Use the accent color to mark the active concept path
- Keep shadows very soft and restrained
- Typography should be clean, compact, and secondary to motion
- The overall feeling should be technical, polished, and calm rather than cute
  or playful

## Deliverables

- A short master `.mp4` video for full-sequence preview
- A matching animated `.gif` preview
- A poster frame for quick inspection
- A segmented output set for embedding into different PPT pages
- A generator script so the animation can be iterated later
