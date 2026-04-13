# PSO Placeholder Insert Pages Design

**Goal:** insert three placeholder onepage explanation slides into the existing page 13-15 PSO loop chapter without breaking the current main flow semantics.

## Why Placeholder Pages

The new topics are explanatory rather than topological:

- how runtime collects PSO data inside the phone side
- what `expand` / `build` actually do and what each intermediate file contains
- how precompile works and why `OpenGL` and `Metal` differ in compile counts

Forcing these topics into the existing loop diagrams would overload the routing and weaken the main story.
So the simplest safe move is to insert onepage explanation slides between the existing flow pages.

## Recommended Numbering

Keep the current page 13-15 semantics, but shift them back by insertion:

- `page_13`: keep current `.ushaderbytecode -> Phone`
- `page_14`: new placeholder onepage about runtime PSO collection
- `page_15`: old current `Phone -> rec.upipelinecache -> Computer`
- `page_16`: new placeholder onepage about `expand / build` and file contents
- `page_17`: old current stable closed-loop page
- `page_18`: new placeholder onepage about precompile and `OpenGL / Metal` compile-count differences

This avoids `13A / 14A / 15A` naming, keeps step navigation simple, and preserves the current flow chapter as the backbone.

## Transition Rule

All three inserted pages should use the same transition grammar:

### Main flow page -> onepage

- shrink and fade the outgoing flow page as a whole
- keep it visible briefly as a background memory plate
- float in the onepage content from center with a calm scale-up

### Onepage -> next main flow page

- shrink and fade the onepage away
- briefly restore the previous flow page static state
- then play the original frame-to-frame motion into the next flow page

This keeps the inserted pages feeling like explanation cards rather than permanent topology changes.

## Visual Form

All three inserted pages should be treated as lightweight explanation boards, not dense diagrams.

Shared layout:

- top title row
- one short thesis sentence under the title
- two or three central content cards
- one bottom comparison or takeaway strip

Shared styling:

- same palette family as the loop chapter
- reduced line density compared with the flow pages
- no attempt to fully mimic the loop routing language
- emphasis through grouping, icon chips, and short labels rather than many arrows

## Page 14 Placeholder

### Topic

How the phone side collects PSO information at runtime.

### Core thesis

`Phone` does not only consume `.ushaderbytecode`; it also records runtime observations that later become `.rec.upipelinecache`.

### Content blocks

Left card:

- runtime collection chain
- `Draw / ShaderHash / State -> rec.upipelinecache`

Right card:

- `OpenGL` vs `Metal / modern gfx api`
- `OpenGL` tends to expose fewer explicit pipeline-state combinations
- `Metal` keeps more pipeline-state combinations explicit

Bottom takeaway:

- `OpenGL` PSO-like record counts are usually lower because more state is implicit or driver-owned
- `Metal` counts are usually higher because pipeline composition is more explicit and combinatorial

## Page 16 Placeholder

### Topic

What `expand` and `build` do, and what each file carries.

### Core thesis

The computer side is not doing magic; it is transforming runtime hashes into stable keys and stable pipeline records.

### Content blocks

Four file cards:

1. `rec.upipelinecache`
   - `ShaderHash + State`
   - `OpenGL` often skews toward the `ShaderHash` side
2. `scl.csv`
   - `ShaderStableKey <-> ShaderHash`
3. `stablepc.csv`
   - `ShaderStableKey + State`
   - `OpenGL` often skews toward the `ShaderStableKey` side
4. `stable.upipelinecache`
   - stabilized build output for later compilation / loading

Bottom takeaway:

- `expand` is the hash-to-stable-key normalization step
- `build` is the stable-record assembly step

## Page 18 Placeholder

### Topic

How precompile happens and why `OpenGL` and `Metal` differ in compile counts.

### Core thesis

Precompile consumes the stabilized cache representation, but the amount of work still depends heavily on API explicitness.

### Content blocks

Left card:

- precompile input
- `stable.upipelinecache`

Center card:

- process
- `StableKey / State -> build pipeline -> precompile`

Right card:

- compile-count comparison
- `OpenGL`: usually fewer compile targets
- `Metal`: usually more compile targets

Bottom takeaway:

- modern explicit APIs cost more upfront compilation work but give stronger control and determinism

## Scope Rules

- keep these pages as placeholder onepages first
- do not force them into the existing geometry harness scoring loop immediately
- do not redesign the current page 13 / 15 / 17 flow topology while inserting them
- prioritize correct chapter sequencing and calm explanatory readability over polish

## Expected Implementation Outcome

After insertion, the chapter should read as:

1. bytecode enters runtime
2. pause and explain runtime collection
3. runtime sends `.rec.upipelinecache` back
4. pause and explain expand / build artifacts
5. stable loop closes
6. pause and explain precompile consequences

That gives the audience one concrete explanation stop after each major phase of the loop.
