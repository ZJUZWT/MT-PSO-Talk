export type VariantId =
  | "bus-clean"
  | "bus-wide"
  | "shared-focus";

export type StoryStepId =
  | "base_formula"
  | "opengl_state_machine"
  | "vulkan_pso"
  | "open_pso"
  | "inline_material"
  | "shared_code";

export type StoryStep = {
  id: StoryStepId;
  label: string;
  caption: string;
  notes: string;
  focusTarget: string;
  timingHint: string;
  intro?: string;
  manuscript?: string;
  codeSample?: string;
  codeLegend?: Array<{label: string; color: string}>;
  focusColorKey?: string;
};

export type Storyboard = {
  storyId: string;
  title: string;
  summary: string;
  steps: StoryStep[];
};
