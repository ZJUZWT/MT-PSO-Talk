import {describe, expect, it} from "vitest";
import {masterStoryboard} from "../../../shared/pso-storyboard";

describe("masterStoryboard", () => {
  it("locks the canonical step order", () => {
    expect(masterStoryboard.steps.map((step) => step.id)).toEqual([
      "base_formula",
      "open_fx",
      "inline_material",
      "shared_code",
    ]);
  });
});
