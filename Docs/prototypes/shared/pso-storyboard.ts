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
      caption: "Start with the irreducible render model before introducing UE-specific structure.",
      notes:
        "Anchor the audience on the stable transform first. Everything later is an expansion of f(x), not a brand-new concept.",
      focusTarget: "f(x)",
      timingHint: "Hold briefly and keep the skeleton centered.",
    },
    {
      id: "open_fx",
      label: "Open f(x)",
      caption: "Expand f(x) into ShaderCode and Pipeline State without losing the original mental map.",
      notes:
        "The frame grows first, then internal structure appears. A and B should fade into context rather than disappear.",
      focusTarget: "ShaderCode + Pipeline State",
      timingHint: "Box expansion first, internal nodes second, connectors last.",
    },
    {
      id: "inline_material",
      label: "Inline material",
      caption: "Show how Material can directly feed ShaderCode in the inline path.",
      notes:
        "Use orthogonal routing and move nodes if space gets tight. The path should read as engineered, not decorative.",
      focusTarget: "Material -> ShaderCode",
      timingHint: "Bring Material in from the side, then reveal the path.",
    },
    {
      id: "shared_code",
      label: "Shared code",
      caption: "Collapse multiple Materials toward a shared ShaderCodeLib and reconnect the main function path.",
      notes:
        "The key is structural compression: many materials converge, shared code centralizes, and runtime use becomes readable again.",
      focusTarget: "ShaderCodeLib",
      timingHint: "Settle into a stable overview state that can hold on a slide.",
    },
  ],
};
