import type {
  StoryStepId,
  VariantId,
} from "../../shared/pso-workbench-types";
import {resolveMotionCanvasStepFrame} from "./embed-config";

export type MotionCanvasEmbedState = {
  stepId: StoryStepId;
  variantId: VariantId;
  initialFrame: number;
};

export function resolveMotionCanvasEmbedState(
  search: string,
): MotionCanvasEmbedState {
  const params = new URLSearchParams(search);
  const requestedStep = params.get("step") as StoryStepId | null;
  const requestedVariant = params.get("variant") as VariantId | null;
  const stepId = requestedStep ?? "base_formula";
  const variantId = requestedVariant ?? "bus-clean";

  return {
    stepId,
    variantId,
    initialFrame: resolveMotionCanvasStepFrame(stepId),
  };
}
