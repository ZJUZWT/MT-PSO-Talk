import {describe, expect, it} from "vitest";
import {createRemotionAdapter} from "./remotionAdapter";

describe("createRemotionAdapter", () => {
  it("maps the current workbench state to player props", () => {
    const adapter = createRemotionAdapter();
    const playerState = adapter.resolvePlayerState({
      libraryId: "remotion",
      variantId: "bus-clean",
      stepId: "open_fx",
    });

    expect(playerState.inputProps).toEqual({variantId: "bus-clean"});
    expect(playerState.initialFrame).toBeGreaterThan(0);
  });
});
