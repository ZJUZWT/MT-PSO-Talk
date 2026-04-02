import {readFileSync} from "node:fs";
import {resolve} from "node:path";
import {describe, expect, it} from "vitest";

const motionCanvasEmbedSource = readFileSync(
  resolve(import.meta.dirname, "../../../motion-canvas-demo/src/embed.ts"),
  "utf8",
);

describe("Motion Canvas embed bridge", () => {
  it("bootstraps the player from the compiled project entry", () => {
    expect(motionCanvasEmbedSource).toContain(
      'import project from "./project?project";',
    );
  });

  it("seeds the player with the selected step frame on first load", () => {
    expect(motionCanvasEmbedSource).toContain("initialSelection.initialFrame");
  });

  it("delegates query parsing to the shared embed-state helper", () => {
    expect(motionCanvasEmbedSource).toContain("resolveMotionCanvasEmbedState");
  });
});
