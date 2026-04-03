import {describe, expect, it} from "vitest";
import {
  diagramLayout,
  diagramLayoutDerived,
} from "../storyboard-data/diagramLayout";

describe("diagramLayout", () => {
  it("uses a full-bleed right-side panel and more restrained arrowheads", () => {
    expect(diagramLayout.panel.width).toBe(diagramLayout.canvas.width);
    expect(diagramLayout.panel.height).toBe(diagramLayout.canvas.height);
    expect(diagramLayout.panel.left).toBe(0);
    expect(diagramLayout.panel.top).toBe(0);
    expect(diagramLayout.arrowheads.remotion.markerWidth).toBeLessThanOrEqual(6);
    expect(diagramLayout.arrowheads.motionCanvas.wire).toBeLessThanOrEqual(10);
  });

  it("routes inline materials into ShaderCode from a lower bus", () => {
    expect(diagramLayout.inlineConnectors.targetXs).toHaveLength(3);
    expect(diagramLayout.inlineConnectors.targetXs[0]).toBeGreaterThan(
      diagramLayout.shaderCode.x - diagramLayout.shaderCode.width / 2,
    );
    expect(diagramLayout.inlineConnectors.targetXs[2]).toBeLessThan(
      diagramLayout.shaderCode.x + diagramLayout.shaderCode.width / 2,
    );
  });

  it("places ShaderCodeLib below ShaderCode with a direct upward link", () => {
    expect(diagramLayout.shaderCodeLib.y).toBeGreaterThan(
      diagramLayout.shaderCode.y,
    );
    expect(diagramLayoutDerived.sharedLink.toShaderY).toBeLessThan(
      diagramLayoutDerived.sharedLink.fromLibY,
    );
  });

  it("raises the inner f(x) row so the boxes sit in a more balanced band", () => {
    expect(diagramLayout.shaderCode.y).toBe(diagramLayout.pipelineState.y);
    expect(diagramLayout.shaderCode.y).toBeLessThan(-60);
  });

  it("keeps shared-code reuse aligned on the ShaderCode centerline", () => {
    expect(diagramLayout.shaderCodeLib.x).toBe(diagramLayout.shaderCode.x);
    expect(diagramLayoutDerived.sharedLink.x).toBe(diagramLayout.shaderCode.x);
  });
});
