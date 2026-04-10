---
name: edge-image-aesthetic-review
description: Use when a visual is currently open in Microsoft Edge and you want an end-to-end aesthetic review that captures the image first and then analyzes it.
---

# Edge Image Aesthetic Review

This skill orchestrates two lower-level skills:

- `fetch-edge-image`
- `analyze-image-aesthetics`

It may also hand durable lessons to `conversation-lesson-harvester` when the user explicitly asks to persist a preference or review rule.

Its job is to turn "look at what is in Edge right now" into a real local artifact plus a structured beauty review.

## When to Use

Use this skill when:

- the user asks to review the look of a page currently visible in Edge
- the visual target is not yet a local file
- you want a repeatable capture-plus-critique workflow

If the user already provided a local image path or an attached image, skip Edge capture and use `analyze-image-aesthetics` directly.

## Required Workflow

1. Resolve the target in Edge.
2. Use `fetch-edge-image` to create a local image artifact.
3. Confirm what was captured:
   - image path
   - active URL
   - whether browser chrome is included
4. Use `analyze-image-aesthetics` on the captured image.
5. If the user asks to remember a visual preference or formalize a review lesson, invoke `conversation-lesson-harvester`.
6. Present the result using the structure in [references/output-template.md](references/output-template.md).

## Required Output

Always include:

- `Capture provenance`
- `Blockers`
- `Overall verdict`
- scored aesthetic table
- `Top 3 fixes`

If capture fails, stop at capture and report the failure instead of pretending analysis succeeded.

## Hard Bans

- Do not critique an unseen Edge page from memory.
- Do not hide capture uncertainty; say exactly what was captured.
- Do not jump into implementation advice before the visual diagnosis is clear.
- Do not merge image capture and image critique into one vague paragraph.
