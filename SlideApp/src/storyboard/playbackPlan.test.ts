import {describe, expect, it} from "vitest";
import type {StoryStepId} from "../storyboard-data/pso-workbench-types";
import {createPlaybackPlan} from "./playbackPlan";

const FRAME_BY_STEP: Record<StoryStepId, number> = {
  base_formula: 18,
  opengl_state_machine: 54,
  vulkan_pso: 90,
  open_pso: 126,
  inline_material: 162,
  shared_code: 198,
};

describe("createPlaybackPlan", () => {
  it("jumps directly to the target frame on first render", () => {
    expect(
      createPlaybackPlan({
        previousStepId: null,
        stepId: "shared_code",
        resolveFrame: (stepId) => FRAME_BY_STEP[stepId],
      }),
    ).toEqual({
      shouldAnimate: false,
      fromFrame: 198,
      toFrame: 198,
      durationMs: 0,
    });
  });

  it("animates forward when moving to the next step", () => {
    expect(
      createPlaybackPlan({
        previousStepId: "opengl_state_machine",
        stepId: "vulkan_pso",
        resolveFrame: (stepId) => FRAME_BY_STEP[stepId],
      }),
    ).toEqual({
      shouldAnimate: true,
      fromFrame: 54,
      toFrame: 90,
      durationMs: 324,
    });
  });

  it("animates backward when moving to an earlier step", () => {
    expect(
      createPlaybackPlan({
        previousStepId: "shared_code",
        stepId: "inline_material",
        resolveFrame: (stepId) => FRAME_BY_STEP[stepId],
      }),
    ).toEqual({
      shouldAnimate: true,
      fromFrame: 198,
      toFrame: 162,
      durationMs: 324,
    });
  });
});
