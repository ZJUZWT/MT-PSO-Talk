import type {
  StoryStepId,
  VariantId,
} from "../storyboard-data/pso-workbench-types";
import {
  resolveRemotionStepFrame,
  type RemotionWorkbenchProps,
} from "../remotion/embed";

type RemotionWorkbenchSelection = {
  variantId: VariantId;
  stepId: StoryStepId;
};

export function createRemotionAdapter() {
  const getFrameForStep = (stepId: StoryStepId) =>
    resolveRemotionStepFrame(stepId);

  const createInputProps = (variantId: VariantId) =>
    ({
      variantId,
    }) satisfies RemotionWorkbenchProps;

  const resolvePlayerState = (state: RemotionWorkbenchSelection) => {
    return {
      inputProps: createInputProps(state.variantId),
      initialFrame: getFrameForStep(state.stepId),
    };
  };

  return {
    id: "remotion" as const,
    resolvePlayerState,
    getFrameForStep,
    createInputProps,
  };
}
