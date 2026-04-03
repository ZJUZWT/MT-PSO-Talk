---
name: technical-talk-animation
description: Use when designing concept animations for technical talks, especially when the result must embed cleanly into PPT or Keynote without looking noisy, overly cinematic, or hard to explain live.
---

# Technical Talk Animation

## Overview

Technical-talk animation is not short-film animation.

The job is to make an idea easier to explain, not to impress with motion. The
best animation for a talk usually feels simpler than what you would build for a
promo video.

## When to Use

Use this skill when:

- a concept diagram needs to evolve over time
- the animation will sit beside speaker notes or bullets in a slide
- the audience needs to understand state changes quickly
- earlier attempts looked busy, overlapping, or hard to follow
- arrows, labels, and boxes are moving at the same time and the result feels
  messy

Do not use this skill when:

- the animation is a standalone trailer or intro sequence
- the main goal is atmosphere rather than explanation

## Core Principle

Lock the skeleton first. Animate emphasis second.

In practice, that means:

- keep major objects on stable positions or a stable grid
- change one idea at a time
- let the audience preserve their mental map across the whole sequence

## Layout Rules

- Reserve a text-safe area for slide copy before drawing the animation
- Prefer a white or near-white background when the result will be embedded in a
  slide deck
- Put the animation inside a bounded stage or card if that helps separate it
  from slide text
- Make the last frame clean enough to pause on and explain verbally

## Motion Rules

- Use smooth easing, not elastic or bouncy easing
- Use scale, position, and opacity as the main motion channels
- Highlight the active concept by enlarging it slightly and reducing emphasis on
  secondary context
- Do not animate every element at once
- If two things change, stage them in sequence rather than in parallel
- For text-heavy speaker-note cards, keep motion on a single axis; vertical deck
  motion reads cleaner than diagonal tosses
- Keep the outgoing card behind the incoming card for the full transition so the
  new page never gets covered mid-flight
- If a pill, chip, or step indicator collapses, animate the close of its inner
  content as well as the shell; shrinking only the outer width reads like a cut
- Start transition state before the first painted frame whenever possible; one
  idle frame between pages is visible in a talk setting

## Arrow Rules

- Keep arrows orthogonal: horizontal and vertical segments only
- Do not let arrows swing diagonally during transitions
- Prefer arrows that appear after boxes settle, not arrows that morph wildly
  while the layout is still moving
- If an arrow endpoint would cause overlap or visual noise, delay the arrow
  fade-in until the target state is stable

## Visual Rules

- Use one emphasis color system, not several competing highlight colors
- Keep non-active elements in neutral grays
- Use rounded boxes, but keep the style technical and restrained
- Shadows should be soft and subtle; if the shadow is noticeable, it is usually
  too strong

## Failure Modes

These are strong warning signs:

- final-state boxes overlap or nearly touch
- arrows cross through labels or boxes
- multiple elements are enlarged at the same time
- the viewer cannot tell what changed in the last second
- the paused frame looks like an in-between rather than a diagram
- the animation looks good only in motion but ugly when paused

## Recommended Workflow

1. Define the final paused diagram first.
2. Place every major node on a stable skeleton.
3. Define the intermediate keyframes without overlap.
4. Add emphasis scaling only after the keyframes read clearly.
5. Add arrows last, and make them orthogonal.
6. Export both a master clip and per-beat clips for slide embedding.

## PPT Embedding Guidance

- Prefer MP4 over GIF for the slide deck
- Set the video to autoplay when the slide opens
- Only auto-advance the slide if the speaking rhythm is fixed
- In most technical talks, autoplay video plus manual slide advance is safer
- Export short beat-sized clips when one long video would outrun the narration

## Quick Checklist

Before calling the animation ready, confirm:

- there is a text-safe region
- the background works on a normal PPT slide
- no keyframe has overlap
- arrows are orthogonal and stable
- only one concept is visually dominant at a time
- the final frame is presentation-worthy on its own
