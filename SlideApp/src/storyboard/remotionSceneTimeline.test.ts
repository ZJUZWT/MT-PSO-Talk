import {describe, expect, it} from "vitest";
import {resolveRemotionSceneWindow} from "../remotion/sceneTimeline";

describe("resolveRemotionSceneWindow", () => {
  it("treats exact step anchors as stable single-step scenes", () => {
    expect(resolveRemotionSceneWindow(54)).toEqual({
      fromStepId: "opengl_state_machine",
      toStepId: "opengl_state_machine",
      fromFrame: 54,
      toFrame: 54,
      progress: 1,
    });
  });

  it("resolves in-between frames as transitions between neighboring storyboard steps", () => {
    expect(resolveRemotionSceneWindow(72)).toEqual({
      fromStepId: "opengl_state_machine",
      toStepId: "vulkan_pso",
      fromFrame: 54,
      toFrame: 90,
      progress: 0.5,
    });
  });

  it("clamps frames after the last anchor to the final stable scene", () => {
    expect(resolveRemotionSceneWindow(214)).toEqual({
      fromStepId: "shared_code",
      toStepId: "shared_code",
      fromFrame: 198,
      toFrame: 198,
      progress: 1,
    });
  });
});
