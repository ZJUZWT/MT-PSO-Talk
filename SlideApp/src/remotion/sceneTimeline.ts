import type {StoryStepId} from "../storyboard-data/pso-workbench-types";
import {resolveRemotionStepFrame} from "./embed";

export const REMOTION_STEP_SEQUENCE: StoryStepId[] = [
  "base_formula",
  "opengl_state_machine",
  "vulkan_pso",
  "open_pso",
  "inline_material",
  "shared_code",
];

export function resolveRemotionSceneWindow(frame: number) {
  const safeFrame = Math.max(0, Math.round(frame));
  const firstStepId = REMOTION_STEP_SEQUENCE[0];
  const firstFrame = resolveRemotionStepFrame(firstStepId);

  if (safeFrame <= firstFrame) {
    return {
      fromStepId: firstStepId,
      toStepId: firstStepId,
      fromFrame: firstFrame,
      toFrame: firstFrame,
      progress: 1,
    };
  }

  for (let index = 1; index < REMOTION_STEP_SEQUENCE.length; index += 1) {
    const stepId = REMOTION_STEP_SEQUENCE[index];
    const stepFrame = resolveRemotionStepFrame(stepId);

    if (safeFrame === stepFrame) {
      return {
        fromStepId: stepId,
        toStepId: stepId,
        fromFrame: stepFrame,
        toFrame: stepFrame,
        progress: 1,
      };
    }

    if (safeFrame < stepFrame) {
      const previousStepId = REMOTION_STEP_SEQUENCE[index - 1];
      const previousFrame = resolveRemotionStepFrame(previousStepId);
      const progress = (safeFrame - previousFrame) / (stepFrame - previousFrame);

      return {
        fromStepId: previousStepId,
        toStepId: stepId,
        fromFrame: previousFrame,
        toFrame: stepFrame,
        progress,
      };
    }
  }

  const lastStepId = REMOTION_STEP_SEQUENCE[REMOTION_STEP_SEQUENCE.length - 1];
  const lastFrame = resolveRemotionStepFrame(lastStepId);

  return {
    fromStepId: lastStepId,
    toStepId: lastStepId,
    fromFrame: lastFrame,
    toFrame: lastFrame,
    progress: 1,
  };
}
