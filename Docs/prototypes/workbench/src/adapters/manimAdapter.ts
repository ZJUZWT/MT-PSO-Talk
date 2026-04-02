import type {StoryStepId} from "../../../shared/pso-workbench-types";

const STEP_REFERENCE_MAP: Record<
  StoryStepId,
  {timeInSeconds: number; note: string}
> = {
  base_formula: {
    timeInSeconds: 0,
    note: "Reference the opening formula frame before the graph starts expanding.",
  },
  open_fx: {
    timeInSeconds: 0.8,
    note: "Pause where the function box has opened and the inner nodes become legible.",
  },
  inline_material: {
    timeInSeconds: 1.6,
    note: "Use the inline material path as a comparison beat, even if the final media is still pending.",
  },
  shared_code: {
    timeInSeconds: 2.2,
    note: "Land on the stabilized shared-code overview so the audience can compare structures.",
  },
};

export function createManimAdapter() {
  return {
    id: "manim" as const,
    kind: "reference" as const,
    isAvailable: false,
    resolveStep(stepId: StoryStepId) {
      return STEP_REFERENCE_MAP[stepId];
    },
  };
}
