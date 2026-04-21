# Page24 Density Design

**Goal:** Densify `page_24` without changing semantics: left remains `包体 / ShaderCode 压缩`, right remains `内存 / UE 中 PSO：LRU + mmap`, and the current large whitespace should be reduced to roughly half of the latest measured margins.

**Approved Direction:** The user chose structural densification (`A`) and explicitly asked to halve the current whitespace budget. Current measured outer margins are `top 110 / right 84 / bottom 60 / left 84`; the new layout should target approximately `top 55 / right 42 / bottom 30 / left 42` while keeping geometry safe.

## Design Summary

- Expand both formal cards outward so they occupy more of the 1280x720 canvas.
- Keep the slide formal and data-driven; do not add casual prose or filler text.
- Enrich density through structure:
  - Left side: turn each compression row into a thicker formal matrix with stronger algorithm / ratio / platform grouping.
  - Right side: split the memory strategy into clearer residency / swap-reload / mapping-carrier bands.
- Preserve page semantics, transition semantics, and the existing page24/page25 continuity.

## Affected Files

- Modify: `SlideApp/src/remotion/pages/Page10Scene.tsx`
- Modify: `SlideApp/src/review/formalPageReviewRegistry.ts`
- Modify: `SlideApp/src/remotion/Composition.test.tsx`
- Modify: `SlideApp/src/storyboard-data/pso-storyboard.ts`
- Modify: `Docs/剧本/24-第二十四页-正式动画.md`

## Acceptance Criteria

- Page24 keeps `包体 / ShaderCode 压缩` on the left and `内存 / UE 中 PSO：LRU + mmap` on the right.
- Measured outer whitespace is materially reduced toward the user’s “halve it” target.
- No new casual or explanatory prose appears inside the formal graphic.
- Formal review surfaces remain explicit for the left-side platform pills and the right-side strategy structure.
- Required checks pass:
  - `npm --prefix SlideApp test -- lateTailGeometry formalPageReviewRegistry pso-storyboard`
  - `python3 scripts/slide-geometry-harness/audit_transition_timings.py --output-dir generated/transition-timing-audit`
  - `python3 scripts/slide-geometry-harness/audit_storyboard_sync.py`
