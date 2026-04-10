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

If browser chrome is included, treat the slide stage as the subject and do not critique the browser frame as part of the design.

## Art Critic

Focus on:

- spacing rhythm
- too loose vs too tight
- left/right and top/bottom balance
- hierarchy and visual focus
- whether the receiver plane actually feels dominant

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

Ignore:

- final decoration
- typography style beyond basic readability
- code quality
