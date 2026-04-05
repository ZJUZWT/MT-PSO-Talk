# SlideApp Asset Cook Transition Design

## Summary

This revision extends the reduced SlideApp storyboard from four steps to five.
The new goal is to bridge from raw graphics API concepts into UE-style asset
thinking without reintroducing the full PSO packaging layers too early.

## Approved Direction

The user-approved semantic chain is:

- page 03: `Raw ShaderCode -> Binary ShaderCode`
- page 04: `Raw ShaderCode -> SPIR-V ShaderCode`
- page 05: `Material -(cook)-> Cooked ShaderCode`

At the same time, page 05 should introduce asset-origin inputs:

- `Mesh -> VertexData`
- `Material -> Cooked ShaderCode`

and then return to the simpler runtime spine:

- `VertexData -> GPU -> Pixels`
- `Cooked ShaderCode -> GPU`

## Visual Intent

### Page 03

Keep the OpenGL setup framing, but rename the shader lane so it clearly reads
as a raw authoring form becoming a compiled binary through `glCompileShader()`.

### Page 04

Keep the Vulkan framing, but rename the shader lane to make the conversion from
raw shader code to `SPIR-V ShaderCode` explicit. This is still a Vulkan page.

### Page 05

Remove the temporary `Description` and `PSO` packaging layers from the visual
center. The page should feel like a controlled return to the simpler
`inputs -> GPU -> pixels` model, except the inputs now come from UE-flavored
assets and cooking:

- `Mesh` becomes the source of `VertexData`
- `Material` becomes the source of `Cooked ShaderCode`

`Mesh` and `Material` should use a visually distinct asset node style so they
read as authored assets, not runtime payloads.

## Labeling Decision

To keep the boxes short enough for the current canvas:

- use stacked two-line labels where needed
- page 03 shader source node becomes:
  - line 1: `Raw`
  - line 2: `ShaderCode`
- page 04 target shader node becomes:
  - line 1: `SPIR-V`
  - line 2: `ShaderCode`
- page 05 cooked result node becomes:
  - line 1: `Cooked`
  - line 2: `ShaderCode`

This preserves semantic precision without forcing overly wide boxes.

## Constraints

- Do not rename earlier pages into UE terms too early.
- Do not keep `Description` or `PSO` on page 05.
- Do not make `Material` look like a runtime object submitted directly to `GPU`.
- Do not collapse `Mesh` and `Material` into the same visual class as
  `VertexData` and cooked shader payloads.
- Preserve the current restrained motion language and spatial continuity.

## Validation Target

The redesign is successful when:

- storyboard data exposes five pages in the expected order
- page 03/04/05 manuscripts match the approved semantic chain
- the composition can render a fifth page
- labels support compact two-line rendering
- asset nodes are visually distinct from runtime nodes
