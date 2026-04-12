# Page 10-14 Loop Story Design

**Goal:** extend the PSO talk from page 09 into a five-page loop that explains cook outputs, runtime collection, stable expansion, and the final asset-device-computer closed loop.

## Narrative Spine

Pages 10-14 should feel like one loop, not five unrelated diagrams.
The emphasis shifts every page:

- `page_10`: computer-side cook outputs and the first answer reveal
- `page_11`: cooked outputs landing on the runtime side
- `page_12`: runtime feeding back `.rec.upipelinecache`
- `page_13`: computer-side expand and stable outputs
- `page_14`: assets, device, and computer all visible to close the loop

The audience should always be able to answer two questions:

1. Which side is the current focus?
2. Which route is newly active on this page?

## Page 10

### Core Story

Page 10 is the answer page after the SharedCode explanation.
It should briefly recall page 05, collapse into the old question mark, pop into an exclamation mark, and then rebuild into the new cook/runtime frame.

### Animation Contract

The transition should read as:

1. briefly show page 05 state
2. shrink attention into the page 05 `?`
3. turn the `?` into a lively `!`
4. let that `!` trigger the new page 10 layout

The `!` is transitional only and should not remain in the final stop frame.

### Final Stop Frame

The final page 10 layout should only keep:

- left computer
- one `cook` hub
- `.shaderbytecode`
- `.scl.csv`
- right runtime frame that borrows the page 02 language and only hints at a phone shell

The old `VertexData -> GPU -> Pixels` chain does not remain literally on the page.
Its semantic role is to morph into the right runtime frame.

## Page 11

### Core Story

Cook outputs begin to connect to the runtime side.
The computer shrinks and the runtime side grows.

### Layout Contract

- keep the same left computer and right runtime poles
- grow the right runtime frame
- connect both `.shaderbytecode` and `.scl.csv` into the runtime side
- the active emphasis is on the incoming runtime usage path, not on the computer

## Page 12

### Core Story

The runtime side now emits `.rec.upipelinecache`.
This is the first visible return leg of the loop.

### Layout Contract

- the runtime frame is still readable but now starts to shrink after the callout
- the upward or outward route carrying `.rec.upipelinecache` should be the one new bold path
- the computer can become small but must still remain visible as the future receiver

## Page 13

### Core Story

The loop reaches the computer side again and expands into stable products.

### Layout Contract

- computer grows back and becomes the main visual anchor
- `cook` grows with it
- an `expand` route is introduced
- `stablepc.csv` and `stable.upipelinecache` become the new emphasized outputs

## Page 14

### Core Story

The system closes the loop.
Assets, device, and computer should all be visible and mutually legible.

### Layout Contract

- computer shrinks from page 13 but stays visible
- phone/runtime grows again
- assets also become visible and larger than before
- the frame should feel like a complete cycle of collection, build, and use

## Visual Language

- left is authoring/build side
- right is runtime/use side
- keep lines orthogonal where possible
- every page should have one dominant focal cluster and one newly activated route
- inactive clusters should shrink rather than vanish unless they would clutter the stop frame
- the phone should use a hybrid language:
  - inner runtime frame derived from page 02
  - outer shell only lightly suggests a phone silhouette

## Implementation Scope

- `page_10` should be implemented as a formal animated scene in the main story flow
- `page_11` to `page_14` can be sketched first, but each needs its own browser-openable sketch URL
- all five new pages should be registered as real steps so controls and review URLs stay honest
