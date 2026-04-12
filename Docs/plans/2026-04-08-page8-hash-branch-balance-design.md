# Page 08 Hash Branch Balance Design

**Goal:** Re-layout slide page 08 so it keeps page 07's InlineCode lookup skeleton, while `PSO Cache` and the two red proof branches read as lower explanation layers instead of hijacking the runtime mainline.

## Approved Direction

The user selected the conservative option:

- keep page 07's left spine and right InlineCode payload recognizable
- do not rebuild page 08 into a new system
- only re-balance the lower `PSO Cache` layer and the two red proof branches
- keep the main lookup path visually stable:
  - `FShader -> FShaderMapResource_InlineCode -> FShaderMapResourceCode -> ShaderEntries[idx] -> Cooked ShaderCode`

## Page Contract

| Subject | Type | Rationale |
| --- | --- | --- |
| Page 07 upper skeleton | Global | This must stay recognizable so page 08 feels like the same zoomed scene, not a fresh diagram. |
| `FShaderMapResourceCode` payload | Node | This remains the receiver plane for the runtime lookup story. |
| `ShaderHashes[idx]` | Node | This is still a secondary pill inside the payload, but page 08 can now highlight why it matters. |
| `PSO Cache` | Node | This is a lower explanation layer. It explains hash metadata usage, but it must not replace the runtime lookup spine. |
| Outside `Material` proof | Node | This is a side proof for "InlineCode scatters code across uasset ownership", not the primary visual focus. |
| Red proof branches | Edge | They must read as evidence links, not a new runtime execution path. |

## Spatial Intent

- The primary channel stays inside the `uasset` upper band.
- `PSO Cache` should breathe below the `uasset` instead of spanning the whole canvas as if it were the new owner of the page.
- The hash branch should rise from `PSO Cache` toward `ShaderHashes[idx]` with simple orthogonal routing and a clear split, but without dominating the page width.
- The outside `Material` proof should stay visually local to `Cooked ShaderCode`.
- Red should stay strong, but only on the proof links and proof badges.

## Risks To Avoid

- making `PSO Cache` so wide or so central that it becomes the new page receiver plane
- making the red dashed hash branch look like the main runtime lookup
- letting the outside `Material` proof collide with the `Cooked ShaderCode` lane
- changing page 07 anchors while "fixing" page 08

## Acceptance Checks

1. The page 07 upper skeleton remains recognizably stable on page 08.
2. `PSO Cache` sits clearly below the `uasset` with visible breathing room.
3. The red hash proof branch is visibly a side explanation connected to `ShaderHashes[idx]`, not the mainline.
4. The outside `Material` proof remains local to `Cooked ShaderCode` and does not compete with `PSO Cache`.
5. The rendered page still passes the project's line-routing benchmark: orthogonal routing, minimal bends, and one obvious mainline.
