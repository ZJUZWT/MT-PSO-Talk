import {describe, expect, it} from "vitest";
import {createRemotionAdapter} from "./remotionAdapter";

describe("createRemotionAdapter", () => {
  it("maps the current workbench state to player props", () => {
    const adapter = createRemotionAdapter();
    const playerState = adapter.resolvePlayerState({
      variantId: "bus-clean",
      stepId: "vulkan_pso",
    });

    expect(playerState.inputProps).toEqual({variantId: "bus-clean"});
    expect(playerState.initialFrame).toBe(90);
  });
});
