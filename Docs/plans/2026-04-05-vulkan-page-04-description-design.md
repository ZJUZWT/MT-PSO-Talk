# Vulkan Page 04 Description Design

**Goal:** Reframe page 04 so it explains Vulkan as "upstream configuration enters a description, a PSO is created from that description, and runtime binding is reduced to PSO -> GPU" instead of looking like an OpenGL diagram with a PSO box inserted afterward.

## Approved Design

| Subject | Type | Rationale |
| --- | --- | --- |
| Page 03 skeleton | Global | Keep the page-03 lower runtime spine and upper band so page 04 still feels like an evolution, not a redraw. |
| `ShaderBinary -> SPIR-V` rename | Node | Page 04 is now a Vulkan page, so the compiled shader artifact should stop using the OpenGL-style `ShaderBinary` wording. |
| `Description` inserted between upper band and `PSO` | Node | The page needs to explain that Vulkan first gathers state and shader artifacts into a description-like input before the PSO exists. |
| `Create` as a relation, not a large extra node | Edge | The semantic step "create the PSO from the description" matters, but a full extra box would overcrowd the inserted layer and weaken page-to-page continuity. |
| Upper inputs feed `Description` with gray lines | Edge | These relationships still exist, but page 04 is no longer emphasizing them as direct runtime GPU calls. |
| `Description -> PSO` is gray | Edge | The create step is conceptually important but should not compete with the single runtime bind call. |
| `PSO -> GPU` remains the only highlighted API call | Edge | This preserves the page’s core contrast: Vulkan reduces the runtime-facing call surface. |
| Left notes copy | Global | The manuscript and notes must explicitly mention `SPIR-V`, the description/create step, and `vkCmdBindPipeline()` so the slide text and diagram tell the same story. |

## Rejected Alternatives

| Subject | Type | Rationale |
| --- | --- | --- |
| Keep `ShaderBinary` on page 04 | Node | It makes the Vulkan page read like a renamed OpenGL page instead of a semantic shift. |
| Add a large standalone `Create PSO` node | Node | It consumes too much of the middle band and makes the inserted-layer stack feel cramped and discontinuous with page 03. |
| Highlight both create and bind in orange | Edge | That weakens the page’s main claim that runtime API pressure is reduced to a single highlighted bind action. |
