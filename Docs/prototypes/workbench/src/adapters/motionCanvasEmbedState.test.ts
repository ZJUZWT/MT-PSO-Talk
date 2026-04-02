import {describe, expect, it} from "vitest";
import {resolveMotionCanvasEmbedState} from "../../../motion-canvas-demo/src/embed-state";

describe("resolveMotionCanvasEmbedState", () => {
  it("maps the requested step to the initial player frame", () => {
    expect(
      resolveMotionCanvasEmbedState("?variant=shared-focus&step=shared_code"),
    ).toMatchObject({
      stepId: "shared_code",
      variantId: "shared-focus",
      initialFrame: 118,
    });
  });

  it("falls back to the canonical opening state", () => {
    expect(resolveMotionCanvasEmbedState("")).toMatchObject({
      stepId: "base_formula",
      variantId: "bus-clean",
      initialFrame: 18,
    });
  });
});
