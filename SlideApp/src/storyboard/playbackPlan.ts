import type {StoryStepId} from "../storyboard-data/pso-workbench-types";

export type PlaybackPlan = {
  shouldAnimate: boolean;
  fromFrame: number;
  toFrame: number;
  durationMs: number;
};

export function createPlaybackPlan(input: {
  previousStepId: StoryStepId | null;
  stepId: StoryStepId;
  resolveFrame: (stepId: StoryStepId) => number;
}): PlaybackPlan {
  const toFrame = input.resolveFrame(input.stepId);

  if (input.previousStepId === null || input.previousStepId === input.stepId) {
    return {
      shouldAnimate: false,
      fromFrame: toFrame,
      toFrame,
      durationMs: 0,
    };
  }

  const fromFrame = input.resolveFrame(input.previousStepId);

  return {
    shouldAnimate: true,
    fromFrame,
    toFrame,
    durationMs: Math.max(280, Math.min(360, Math.abs(toFrame - fromFrame) * 9)),
  };
}
