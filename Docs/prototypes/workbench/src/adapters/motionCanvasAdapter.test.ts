import {describe, expect, it} from "vitest";
import {createMotionCanvasAdapter} from "./motionCanvasAdapter";

describe("createMotionCanvasAdapter", () => {
  it("builds an embed url with the active variant and step", () => {
    const adapter = createMotionCanvasAdapter("http://127.0.0.1:4175");
    const url = adapter.getEmbedUrl({
      libraryId: "motion-canvas",
      variantId: "bus-clean",
      stepId: "shared_code",
    });

    expect(url).toContain("/embed.html?");
    expect(url).toContain("variant=bus-clean");
    expect(url).toContain("step=shared_code");
  });
});
