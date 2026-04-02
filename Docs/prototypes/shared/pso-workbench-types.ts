export type LibraryId = "remotion" | "motion-canvas" | "manim";

export type VariantId =
  | "bus-clean"
  | "bus-wide"
  | "shared-focus"
  | "reference";

export type StoryStepId =
  | "base_formula"
  | "open_fx"
  | "inline_material"
  | "shared_code";

export type StoryStep = {
  id: StoryStepId;
  label: string;
  caption: string;
  notes: string;
  focusTarget: string;
  timingHint: string;
};

export type Storyboard = {
  storyId: string;
  title: string;
  summary: string;
  steps: StoryStep[];
};
