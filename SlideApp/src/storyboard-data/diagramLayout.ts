export const diagramLayout = {
  canvas: {
    width: 1280,
    height: 720,
  },
  panel: {
    width: 1280,
    height: 720,
    left: 0,
    top: 0,
    radius: 32,
  },
  arrowGap: 10,
  fx: {
    y: -110,
    baseWidth: 160,
    expandedWidth: 496,
    baseHeight: 74,
    expandedHeight: 248,
    collapsedRadius: 30,
    expandedRadius: 40,
    labelLift: -84,
  },
  nodes: {
    width: 84,
    height: 62,
    aBaseX: -300,
    aExpandedX: -356,
    bBaseX: 300,
    bExpandedX: 356,
  },
  shaderCode: {
    x: -111,
    y: -70,
    width: 182,
    height: 84,
  },
  pipelineState: {
    x: 111,
    y: -70,
    width: 182,
    height: 84,
  },
  materials: {
    x: -350,
    width: 152,
    height: 54,
    ys: [112, 186, 260] as const,
  },
  inlineConnectors: {
    targetXs: [-174, -116, -58] as const,
  },
  shaderCodeLib: {
    x: -111,
    y: 182,
    width: 188,
    height: 72,
  },
  sharedCollection: {
    busX: -240,
  },
  arrowheads: {
    remotion: {
      markerWidth: 6,
      markerHeight: 6,
      refX: 4.8,
      refY: 3,
    },
    motionCanvas: {
      wire: 9,
      accent: 10,
    },
  },
} as const;

export const shaderCodeBottomY =
  diagramLayout.shaderCode.y + diagramLayout.shaderCode.height / 2 + 4;

export const shaderCodeLibTopY =
  diagramLayout.shaderCodeLib.y - diagramLayout.shaderCodeLib.height / 2 - 2;

export const diagramLayoutDerived = {
  shaderCodeBottomY,
  shaderCodeLibTopY,
  materialRightX:
    diagramLayout.materials.x + diagramLayout.materials.width / 2,
  shaderCodeLeftX:
    diagramLayout.shaderCode.x - diagramLayout.shaderCode.width / 2,
  shaderCodeRightX:
    diagramLayout.shaderCode.x + diagramLayout.shaderCode.width / 2,
  shaderCodeLibLeftX:
    diagramLayout.shaderCodeLib.x - diagramLayout.shaderCodeLib.width / 2,
  sharedLink: {
    x: diagramLayout.shaderCodeLib.x,
    fromLibY: shaderCodeLibTopY - diagramLayout.arrowGap,
    toShaderY: shaderCodeBottomY + diagramLayout.arrowGap,
  },
} as const;
