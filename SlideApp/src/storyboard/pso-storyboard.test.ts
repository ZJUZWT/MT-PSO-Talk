import {describe, expect, it} from "vitest";
import {masterStoryboard} from "../storyboard-data/pso-storyboard";

describe("masterStoryboard", () => {
  it("locks the canonical step order", () => {
    expect(masterStoryboard.steps.map((step) => step.id)).toEqual([
      "base_formula",
      "opengl_state_machine",
      "vulkan_pso",
      "open_pso",
      "inline_material",
      "shared_code",
    ]);
  });

  it("exposes lecture content for the new steps", () => {
    const sampleStep = masterStoryboard.steps.find(
      (step) => step.codeSample !== undefined && step.codeSample.length > 0,
    );

    expect(sampleStep).toBeDefined();
    expect(sampleStep?.focusColorKey).toBeDefined();
    expect(sampleStep?.intro).toBeDefined();
  });
});
