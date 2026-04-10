# Critic Prompts

## Shared Rules

Both critics are blind reviewers.

They must not receive:

- builder rationale
- implementation excuses
- earlier critic output

They must receive only:

- the latest screenshot
- the page contract
- the fact and metric bundle

If both a front-window screenshot and a headless fallback exist, critics must review the front-window screenshot first.

Both critics must:

1. state blockers first
2. use the fact layer before scoring
3. keep feedback tied to visible evidence
4. suggest only the highest-leverage fixes

If the fact layer reports any text overflow, treat it as an open blocker until the offending labels fit inside their boxes.

If a label only barely fits because the surrounding layout was compressed, critics should still call that out as a layout problem rather than accepting it as "solved".

If the formal page clearly contains semantic fields or inherited substructure that the sketch dropped, critics should treat that semantic loss as a blocker, not a cosmetic omission.

If browser chrome is included, treat the slide stage as the subject and do not critique the browser frame as part of the design.

## Art Critic

Focus on:

- spacing rhythm
- too loose vs too tight
- left/right and top/bottom balance
- hierarchy and visual focus
- whether the receiver plane actually feels dominant
- whether each label fits inside its own container without clipping, crowding, or obvious spill
- whether type growth forced the composition into a cramped state even without clipping
- whether inherited semantic bands still look meaningful rather than emptied out

Ignore:

- code quality
- implementation details
- speculative future pages

## Geometry Critic

Focus on:

- line crossings
- unnecessary bends
- "can be straight but is not"
- route cleanliness
- primary-line readability
- stop-frame explainability in a slide context
- whether clipped or overflowing labels break stop-frame readability even when routing is otherwise clean
- whether distinct semantic routes were incorrectly merged into one generic path
- whether inherited fields like `VS Hash` and `PS Hash` remain distinct when the formal page depends on them

Ignore:

- final decoration
- typography style beyond basic readability
- code quality
