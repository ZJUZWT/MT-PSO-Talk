import type {StoryStepId} from "../../shared/pso-workbench-types";

export const MOTION_CANVAS_PLAYER_CONFIG = {
  width: 1280,
  height: 720,
  fps: 60,
  background: "#f7f2e9",
  stepFrameMap: {
    base_formula: 18,
    open_fx: 54,
    inline_material: 78,
    shared_code: 118,
  } satisfies Record<StoryStepId, number>,
} as const;

export function resolveMotionCanvasStepFrame(stepId: StoryStepId): number {
  return MOTION_CANVAS_PLAYER_CONFIG.stepFrameMap[stepId];
}
