import {describe, expect, it} from "vitest";
import {createManimAdapter} from "./manimAdapter";

describe("createManimAdapter", () => {
  it("maps canonical steps to reference timestamps", () => {
    const adapter = createManimAdapter();

    expect(adapter.resolveStep("shared_code").timeInSeconds).toBeGreaterThan(0);
  });
});
