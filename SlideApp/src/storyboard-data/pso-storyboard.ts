import type {Storyboard} from "./pso-workbench-types";

export const masterStoryboard: Storyboard = {
  storyId: "pso-concept",
  title: "PSO Concept Evolution",
  summary:
    "A browser workbench for comparing how different animation runtimes explain the same PSO storyline.",
  steps: [
    {
      id: "base_formula",
      label: "A -> f(x) -> B",
      caption:
        "Start with the irreducible render model before introducing UE-specific structure.",
      notes:
        "Anchor the audience on the stable transform first. Everything later is an expansion of f(x), not a brand-new concept.",
      focusTarget: "f(x)",
      timingHint: "Hold briefly and keep the skeleton centered.",
      intro:
        "Input moves through a single function so we can focus on what PSO will later refine.",
      manuscript:
        "Think of this first page as the book cover: one function, one concept, and one goal—turn inputs into pixels.",
      focusColorKey: "fx",
    },
    {
      id: "opengl_state_machine",
      label: "OpenGL state machine",
      caption:
        "OpenGL draws pixels by manually feeding shader + fixed-function chunks.",
      notes:
        "Describe how shader objects, blend state, and rasterization state must be bound one at a time when OpenGL drives the pipeline.",
      focusTarget: "OpenGL state",
      timingHint: "Show fragmented modules that never coalesce.",
      intro:
        "OpenGL lacks a single PSO, so applications layer shader and plumbing state piece by piece.",
      manuscript:
        "Use two colors in the code block: one for shader stage setups and one for fixed pipeline states. Each call is independent.",
      codeSample: [
        "glUseProgram(shaderProgram); // shader stage",
        "glBindVertexArray(vao);",
        "glEnable(GL_DEPTH_TEST); // depth state",
        "glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // blend state",
        "glDrawElements(...);",
      ].join("\n"),
      codeLegend: [
        {label: "Shader stage binds", color: "--accent"},
        {label: "Fixed pipeline tweaks", color: "--muted"},
      ],
      focusColorKey: "opengl",
    },
    {
      id: "vulkan_pso",
      label: "Vulkan PSO",
      caption:
        "Vulkan surfaces a create-info structure that bundles shader + fixed states into a single PSO.",
      notes:
        "Emphasize the single `VkGraphicsPipelineCreateInfo` declaration and how it captures shader stages, blend, rasterization, and depth/stencil state together.",
      focusTarget: "Vulkan pipeline",
      timingHint: "Unfold the big capsule in one reveal.",
      intro:
        "Vulkan asks you to describe your pipeline in one go so the driver can compile it ahead of time.",
      manuscript:
        "Point out that the just-in-time PSO creation streamlines the earlier OpenGL mess into one orange concept.",
      codeSample: [
        "VkGraphicsPipelineCreateInfo pipelineInfo = {};",
        "pipelineInfo.stageCount = shaderStageCount;",
        "pipelineInfo.pStages = shaderStages; // shaders",
        "pipelineInfo.pRasterizationState = &rasterState;",
        "pipelineInfo.pColorBlendState = &blendState;",
        "vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);",
      ].join("\n"),
      codeLegend: [{label: "Shader + fixed states", color: "--accent"}],
      focusColorKey: "vulkan",
    },
    {
      id: "open_pso",
      label: "Open PSO",
      caption:
        "Unpack the PSO capsule into shader code and pipeline state for teaching clarity.",
      notes:
        "Explain how shared shader code and pipeline configuration form the body of a PSO.",
      focusTarget: "ShaderCode + Pipeline State",
      timingHint: "Expand the capsule, then show the connectors.",
      intro:
        "The PSO you just built now splits back into shader code and the supporting pipeline records.",
      manuscript:
        "This page is a gentle reminder: even PSOs unfold into shader code plus fixed states.",
      focusColorKey: "open_pso",
    },
    {
      id: "inline_material",
      label: "Inline material",
      caption: "Show how Material can directly feed ShaderCode in the inline path.",
      notes:
        "Use orthogonal routing and move nodes if space gets tight. The path should read as engineered, not decorative.",
      focusTarget: "Material -> ShaderCode",
      timingHint: "Bring Material in from the side, then reveal the path.",
      intro:
        "A Material can still trickle straight into shader code when sharing is not present yet.",
      manuscript:
        "Highlight the inline paths with a single arrow for clarity and keep the rest muted.",
      focusColorKey: "inline_material",
    },
    {
      id: "shared_code",
      label: "Shared shader code",
      caption:
        "Collapse multiple Materials toward a shared ShaderCodeLib and reconnect the main function path.",
      notes:
        "The key is structural compression: many materials converge, shared code centralizes, and runtime use becomes readable again.",
      focusTarget: "ShaderCodeLib",
      timingHint: "Settle into a stable overview state that can hold on a slide.",
      intro:
        "Shared code lenses show how material variants converge onto reusable shader libraries.",
      manuscript:
        "Stack the material cards from left-bottom to right-top and feed one clean line into ShaderCodeLib.",
      focusColorKey: "shared_code",
    },
  ],
};
