# Page 32 / 33 Merge Design

## Goal

Remove `page_33` as a standalone closing step and fold its `《逍遥游》` quote into `page_32`, so the deck ends on one combined epilogue page.

## Scope

- Keep `page_31` unchanged as the harness page.
- Turn `page_32` into the final page.
- Put the `《逍遥游》` quote in the upper half of `page_32`.
- Put two recommendation groups below the quote.
- Delete `《反杜林论》` and `重读资本论`.
- Rename `马克思主义哲学` to `人类高质量思政课`.

## Layout Decision

- Upper zone: one quote module centered across the page width.
- Lower-left zone: one recommendation group for books / course.
- Lower-right zone: one recommendation group for games.
- The lower groups sit directly below the quote, not as separate full-page sections.

## Data / Timeline Decision

- Remove `page_33` from storyboard steps, step type unions, remotion step sequence, frame map, and review registries.
- Keep `page_32` as the last visible step and reuse the existing `page_31 -> page_32` handoff.
- Do not introduce a new trailing transition after `page_32`.

## Verification Targets

- `page_32` render includes the quote plus the merged recommendation layout.
- `page_33` no longer appears in storyboard, remotion timeline, or formal review registries.
- Transition audits and storyboard sync remain clean after the merge.
