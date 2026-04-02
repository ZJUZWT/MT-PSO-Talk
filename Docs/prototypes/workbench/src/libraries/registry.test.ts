import {describe, expect, it} from "vitest";
import {resolveLibraryState} from "./registry";

describe("resolveLibraryState", () => {
  it("falls back to the library default variant", () => {
    const resolved = resolveLibraryState({
      libraryId: "manim",
      variantId: "bus-wide",
      stepId: "shared_code",
    });

    expect(resolved.variantId).toBe("reference");
  });

  it("falls back to the nearest supported step", () => {
    const resolved = resolveLibraryState({
      libraryId: "manim",
      variantId: "reference",
      stepId: "inline_material",
    });

    expect(resolved.stepId).toBe("shared_code");
  });
});
