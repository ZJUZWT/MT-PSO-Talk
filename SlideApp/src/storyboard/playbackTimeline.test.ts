import {describe, expect, it} from "vitest";
import {resolvePlaybackFrame} from "../storyboard-data/playbackTimeline";

describe("resolvePlaybackFrame", () => {
  it("keeps the midpoint visually in between instead of rushing to the target", () => {
    expect(
      resolvePlaybackFrame({
        fromFrame: 18,
        toFrame: 54,
        durationMs: 324,
        elapsedMs: 162,
      }),
    ).toBe(36);
  });

  it("clamps to the destination once the transition duration is over", () => {
    expect(
      resolvePlaybackFrame({
        fromFrame: 18,
        toFrame: 54,
        durationMs: 324,
        elapsedMs: 648,
      }),
    ).toBe(54);
  });

  it("supports backward travel with the same linear timing", () => {
    expect(
      resolvePlaybackFrame({
        fromFrame: 198,
        toFrame: 162,
        durationMs: 324,
        elapsedMs: 162,
      }),
    ).toBe(180);
  });
});
