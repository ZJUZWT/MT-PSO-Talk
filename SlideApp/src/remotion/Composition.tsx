import React from "react";
import {AbsoluteFill, useCurrentFrame} from "remotion";
import type {
  StoryStepId,
  VariantId,
} from "../storyboard-data/pso-workbench-types";
import {resolveRemotionSceneWindow} from "./sceneTimeline";
import type {RemotionWorkbenchProps} from "./embed";

const COLORS = {
  panel: "#fbf8f2",
  panelStroke: "#dfd7ca",
  wire: "#55606c",
  wireSoft: "#8f9aa6",
  ink: "#25313d",
  muted: "#6d7884",
  surface: "#fffdf8",
  surfaceAlt: "#f8f2ea",
};

const VARIANT_THEME: Record<
  VariantId,
  {accent: string; accentSurface: string; accentSoft: string}
> = {
  "bus-clean": {
    accent: "#ca6f45",
    accentSurface: "#f3e1d5",
    accentSoft: "#f7ebe2",
  },
  "bus-wide": {
    accent: "#a56d4f",
    accentSurface: "#efe1d5",
    accentSoft: "#f4ebe3",
  },
  "shared-focus": {
    accent: "#b85f3c",
    accentSurface: "#f1ddd1",
    accentSoft: "#f8ede6",
  },
};

const PANEL = {
  width: 1280,
  height: 720,
  radius: 32,
};

const CORE = {
  inputX: -356,
  outputX: 356,
  busY: -6,
  nodeWidth: 88,
  nodeHeight: 62,
};

const OPENGL_LAYOUT = {
  input: {x: -426, y: 126, width: 88, height: 62},
  output: {x: 426, y: 126, width: 88, height: 62},
  busStartX: -338,
  busEndX: 338,
  busY: 126,
  shards: [
    {label: "Shader Program", x: -226, y: -12, width: 194, height: 78},
    {label: "Raster / Vertex", x: 0, y: 26, width: 186, height: 72},
    {label: "Depth / Blend", x: 226, y: -12, width: 180, height: 78},
  ],
} as const;

const OPEN_PSO = {
  shell: {x: 0, y: 4, width: 520, height: 244},
  shaderCode: {x: -122, y: 36, width: 182, height: 84},
  pipelineState: {x: 120, y: 36, width: 194, height: 84},
  materials: [
    {label: "Material A", x: -394, y: 126, width: 150, height: 52},
    {label: "Material B", x: -394, y: 196, width: 150, height: 52},
    {label: "Material C", x: -394, y: 266, width: 150, height: 52},
  ],
  inlineTargetXs: [-182, -122, -62] as const,
  shaderCodeLib: {x: -122, y: 206, width: 192, height: 68},
  sharedBusX: -252,
};

const SHARED_MATERIAL_STACK = [
  {label: "Material A", x: -420, y: 246, width: 150, height: 52},
  {label: "Material B", x: -386, y: 192, width: 150, height: 52},
  {label: "Material C", x: -352, y: 138, width: 150, height: 52},
] as const;

const clamp01 = (value: number) => Math.max(0, Math.min(1, value));
const easeOut = (value: number) => 1 - (1 - clamp01(value)) ** 4;

function centerToTopLeft(x: number, y: number, width: number, height: number) {
  return {
    left: PANEL.width / 2 + x - width / 2,
    top: PANEL.height / 2 + y - height / 2,
  };
}

function resolveSceneWeight(
  stepId: StoryStepId,
  sceneWindow: ReturnType<typeof resolveRemotionSceneWindow>,
) {
  if (sceneWindow.fromStepId === stepId && sceneWindow.toStepId === stepId) {
    return 1;
  }

  if (sceneWindow.fromStepId === stepId) {
    return 1 - sceneWindow.progress;
  }

  if (sceneWindow.toStepId === stepId) {
    return sceneWindow.progress;
  }

  return 0;
}

function resolveSceneTransform(
  stepId: StoryStepId,
  sceneWindow: ReturnType<typeof resolveRemotionSceneWindow>,
) {
  if (sceneWindow.fromStepId === stepId && sceneWindow.toStepId === stepId) {
    return "translate3d(0, 0, 0) scale(1)";
  }

  if (sceneWindow.fromStepId === stepId) {
    const eased = easeOut(sceneWindow.progress);
    return `translate3d(0, ${-14 * eased}px, 0) scale(${1 - eased * 0.018})`;
  }

  if (sceneWindow.toStepId === stepId) {
    const eased = easeOut(sceneWindow.progress);
    return `translate3d(0, ${18 * (1 - eased)}px, 0) scale(${0.982 + eased * 0.018})`;
  }

  return "translate3d(0, 0, 0) scale(1)";
}

type BoxProps = {
  label: string;
  x: number;
  y: number;
  width: number;
  height: number;
  fill?: string;
  stroke?: string;
  color?: string;
  fontSize?: number;
  fontWeight?: number;
  radius?: number;
  textAlign?: "center" | "left";
  paddingInline?: number;
};

const Box: React.FC<BoxProps> = ({
  label,
  x,
  y,
  width,
  height,
  fill = COLORS.surface,
  stroke = COLORS.wire,
  color = COLORS.ink,
  fontSize = 24,
  fontWeight = 600,
  radius,
  textAlign = "center",
  paddingInline = 0,
}) => {
  const position = centerToTopLeft(x, y, width, height);

  return (
    <div
      style={{
        position: "absolute",
        ...position,
        width,
        height,
        paddingInline,
        borderRadius: radius ?? Math.min(height / 2, 28),
        border: `3px solid ${stroke}`,
        background: fill,
        boxSizing: "border-box",
        display: "flex",
        alignItems: "center",
        justifyContent: textAlign === "center" ? "center" : "flex-start",
        color,
        fontFamily: '"Avenir Next", "Helvetica Neue", sans-serif',
        fontSize,
        fontWeight,
        letterSpacing: "-0.03em",
        textAlign,
      }}
    >
      {label}
    </div>
  );
};

const Caption: React.FC<{
  label: string;
  x: number;
  y: number;
  color?: string;
  fontSize?: number;
  fontWeight?: number;
}> = ({label, x, y, color = COLORS.muted, fontSize = 22, fontWeight = 600}) => {
  const position = centerToTopLeft(x, y, 10, 10);

  return (
    <div
      style={{
        position: "absolute",
        ...position,
        transform: "translate(-50%, -50%)",
        color,
        fontFamily: '"Avenir Next", "Helvetica Neue", sans-serif',
        fontSize,
        fontWeight,
        letterSpacing: "-0.02em",
        whiteSpace: "nowrap",
      }}
    >
      {label}
    </div>
  );
};

const Pill: React.FC<{
  label: string;
  x: number;
  y: number;
  width: number;
  fill: string;
  stroke: string;
  color?: string;
}> = ({label, x, y, width, fill, stroke, color = COLORS.ink}) => (
  <Box
    label={label}
    x={x}
    y={y}
    width={width}
    height={40}
    fill={fill}
    stroke={stroke}
    color={color}
    fontSize={18}
    fontWeight={700}
  />
);

type PathProps = {
  points: Array<[number, number]>;
  stroke?: string;
  strokeWidth?: number;
  markerId?: string;
  opacity?: number;
};

const Path: React.FC<PathProps> = ({
  points,
  stroke = COLORS.wire,
  strokeWidth = 4,
  markerId,
  opacity = 1,
}) => {
  const pointString = points
    .map(([x, y]) => `${PANEL.width / 2 + x},${PANEL.height / 2 + y}`)
    .join(" ");

  return (
    <polyline
      points={pointString}
      fill="none"
      stroke={stroke}
      strokeWidth={strokeWidth}
      strokeLinecap="round"
      strokeLinejoin="round"
      opacity={opacity}
      markerEnd={markerId ? `url(#${markerId})` : undefined}
    />
  );
};

const SceneSvg: React.FC<{
  accent: string;
  children: React.ReactNode;
}> = ({accent, children}) => (
  <svg
    width={PANEL.width}
    height={PANEL.height}
    style={{position: "absolute", inset: 0, overflow: "visible"}}
  >
    <defs>
      <marker
        id="wire-arrow"
        markerWidth="5"
        markerHeight="5"
        refX="4.1"
        refY="2.5"
        orient="auto"
        markerUnits="strokeWidth"
      >
        <path d="M0,0 L5,2.5 L0,5 z" fill={COLORS.wire} />
      </marker>
      <marker
        id="accent-arrow"
        markerWidth="5"
        markerHeight="5"
        refX="4.1"
        refY="2.5"
        orient="auto"
        markerUnits="strokeWidth"
      >
        <path d="M0,0 L5,2.5 L0,5 z" fill={accent} />
      </marker>
    </defs>
    {children}
  </svg>
);

const SceneLayer: React.FC<{
  stepId: StoryStepId;
  sceneWindow: ReturnType<typeof resolveRemotionSceneWindow>;
  children: React.ReactNode;
}> = ({stepId, sceneWindow, children}) => {
  const weight = resolveSceneWeight(stepId, sceneWindow);

  if (weight <= 0) {
    return null;
  }

  return (
    <div
      style={{
        position: "absolute",
        inset: 0,
        opacity: weight,
        transform: resolveSceneTransform(stepId, sceneWindow),
        transformOrigin: "50% 50%",
      }}
    >
      {children}
    </div>
  );
};

function renderInputAndOutputNodes() {
  return (
    <>
      <Box
        label="A"
        x={CORE.inputX}
        y={CORE.busY}
        width={CORE.nodeWidth}
        height={CORE.nodeHeight}
        fontSize={34}
        fontWeight={650}
      />
      <Box
        label="B"
        x={CORE.outputX}
        y={CORE.busY}
        width={CORE.nodeWidth}
        height={CORE.nodeHeight}
        fontSize={34}
        fontWeight={650}
      />
    </>
  );
}

const BaseFormulaScene: React.FC<{accent: string; accentSurface: string}> = ({
  accent,
  accentSurface,
}) => (
  <>
    <SceneSvg accent={accent}>
      <Path
        points={[
          [CORE.inputX + CORE.nodeWidth / 2 + 10, CORE.busY],
          [-106, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
      <Path
        points={[
          [106, CORE.busY],
          [CORE.outputX - CORE.nodeWidth / 2 - 10, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
    </SceneSvg>

    {renderInputAndOutputNodes()}

    <Box
      label="f(x)"
      x={0}
      y={CORE.busY}
      width={168}
      height={78}
      fill={accentSurface}
      stroke={accent}
      fontSize={34}
      fontWeight={650}
    />

    <Caption label="Render Input" x={CORE.inputX} y={54} />
    <Caption label="Pixel Output" x={CORE.outputX} y={54} />
  </>
);

const OpenGlScene: React.FC<{
  accent: string;
  accentSoft: string;
}> = ({accent, accentSoft}) => (
  <>
    <SceneSvg accent={accent}>
      <Path
        points={[
          [OPENGL_LAYOUT.input.x + OPENGL_LAYOUT.input.width / 2 + 10, OPENGL_LAYOUT.busY],
          [OPENGL_LAYOUT.busStartX, OPENGL_LAYOUT.busY],
        ]}
        markerId="wire-arrow"
      />
      <Path
        points={[
          [OPENGL_LAYOUT.busStartX, OPENGL_LAYOUT.busY],
          [OPENGL_LAYOUT.busEndX, OPENGL_LAYOUT.busY],
        ]}
        stroke={COLORS.wire}
        strokeWidth={4}
      />
      <Path
        points={[
          [OPENGL_LAYOUT.busEndX, OPENGL_LAYOUT.busY],
          [OPENGL_LAYOUT.output.x - OPENGL_LAYOUT.output.width / 2 - 10, OPENGL_LAYOUT.busY],
        ]}
        markerId="wire-arrow"
      />

      {OPENGL_LAYOUT.shards.map((card) => (
        <Path
          key={`${card.label}-tap`}
          points={[
            [card.x, card.y + card.height / 2 + 8],
            [card.x, OPENGL_LAYOUT.busY - 18],
            [card.x, OPENGL_LAYOUT.busY],
          ]}
          stroke={COLORS.wire}
          strokeWidth={4}
          markerId="wire-arrow"
        />
      ))}
    </SceneSvg>

    <Box
      label="A"
      x={OPENGL_LAYOUT.input.x}
      y={OPENGL_LAYOUT.input.y}
      width={OPENGL_LAYOUT.input.width}
      height={OPENGL_LAYOUT.input.height}
      fontSize={34}
      fontWeight={650}
    />
    <Box
      label="B"
      x={OPENGL_LAYOUT.output.x}
      y={OPENGL_LAYOUT.output.y}
      width={OPENGL_LAYOUT.output.width}
      height={OPENGL_LAYOUT.output.height}
      fontSize={34}
      fontWeight={650}
    />

    <Pill
      label="OpenGL state machine"
      x={0}
      y={-146}
      width={258}
      fill={accentSoft}
      stroke={accent}
    />

    {OPENGL_LAYOUT.shards.map((card) => (
      <Box
        key={card.label}
        label={card.label}
        x={card.x}
        y={card.y}
        width={card.width}
        height={card.height}
        fill={COLORS.surface}
        stroke={COLORS.wire}
        fontSize={26}
        fontWeight={620}
      />
    ))}

    <Caption label="Manual state taps feed the draw" x={0} y={208} />
  </>
);

const VulkanScene: React.FC<{
  accent: string;
  accentSurface: string;
  accentSoft: string;
}> = ({accent, accentSurface, accentSoft}) => (
  <>
    <SceneSvg accent={accent}>
      <Path
        points={[
          [CORE.inputX + CORE.nodeWidth / 2 + 10, CORE.busY],
          [-228, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
      <Path
        points={[
          [228, CORE.busY],
          [CORE.outputX - CORE.nodeWidth / 2 - 10, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
    </SceneSvg>

    {renderInputAndOutputNodes()}

    <Box
      label="Vulkan PSO"
      x={0}
      y={CORE.busY}
      width={454}
      height={188}
      fill={accentSurface}
      stroke={accent}
      fontSize={34}
      fontWeight={650}
    />

    <Box
      label="Shader stages"
      x={-118}
      y={46}
      width={166}
      height={50}
      fill={COLORS.surface}
      stroke={accent}
      fontSize={23}
      fontWeight={620}
    />
    <Box
      label="Fixed states"
      x={118}
      y={46}
      width={166}
      height={50}
      fill={COLORS.surface}
      stroke={accent}
      fontSize={23}
      fontWeight={620}
    />

    <Pill
      label="VkGraphicsPipelineCreateInfo"
      x={0}
      y={-124}
      width={320}
      fill={accentSoft}
      stroke={accent}
    />
  </>
);

const OpenPsoScene: React.FC<{
  accent: string;
  accentSurface: string;
}> = ({accent, accentSurface}) => (
  <>
    <SceneSvg accent={accent}>
      <Path
        points={[
          [CORE.inputX + CORE.nodeWidth / 2 + 10, CORE.busY],
          [OPEN_PSO.shell.x - OPEN_PSO.shell.width / 2 - 10, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
      <Path
        points={[
          [OPEN_PSO.shell.x + OPEN_PSO.shell.width / 2 + 10, CORE.busY],
          [CORE.outputX - CORE.nodeWidth / 2 - 10, CORE.busY],
        ]}
        markerId="wire-arrow"
      />
    </SceneSvg>

    {renderInputAndOutputNodes()}

    <Box
      label=""
      x={OPEN_PSO.shell.x}
      y={OPEN_PSO.shell.y}
      width={OPEN_PSO.shell.width}
      height={OPEN_PSO.shell.height}
      fill={accentSurface}
      stroke={accent}
      fontSize={34}
      fontWeight={650}
      radius={42}
    />

    <Caption label="PSO" x={0} y={-78} color={COLORS.ink} fontSize={30} fontWeight={700} />

    <Box
      label="ShaderCode"
      x={OPEN_PSO.shaderCode.x}
      y={OPEN_PSO.shaderCode.y}
      width={OPEN_PSO.shaderCode.width}
      height={OPEN_PSO.shaderCode.height}
      fontSize={28}
      fontWeight={620}
    />
    <Box
      label="Pipeline State"
      x={OPEN_PSO.pipelineState.x}
      y={OPEN_PSO.pipelineState.y}
      width={OPEN_PSO.pipelineState.width}
      height={OPEN_PSO.pipelineState.height}
      fontSize={26}
      fontWeight={620}
    />
  </>
);

const InlineMaterialScene: React.FC<{
  accent: string;
  accentSurface: string;
}> = ({accent, accentSurface}) => {
  const shaderCodeBottomY =
    OPEN_PSO.shaderCode.y + OPEN_PSO.shaderCode.height / 2 + 4;
  const materialRightX =
    OPEN_PSO.materials[0].x + OPEN_PSO.materials[0].width / 2 + 4;

  return (
    <>
      <SceneSvg accent={accent}>
        <Path
          points={[
            [CORE.inputX + CORE.nodeWidth / 2 + 10, CORE.busY],
            [OPEN_PSO.shell.x - OPEN_PSO.shell.width / 2 - 10, CORE.busY],
          ]}
          markerId="wire-arrow"
        />
        <Path
          points={[
            [OPEN_PSO.shell.x + OPEN_PSO.shell.width / 2 + 10, CORE.busY],
            [CORE.outputX - CORE.nodeWidth / 2 - 10, CORE.busY],
          ]}
          markerId="wire-arrow"
        />

        {OPEN_PSO.materials.map((material, index) => {
          const targetX = OPEN_PSO.inlineTargetXs[index];
          const stroke = index === 1 ? accent : COLORS.wire;
          const markerId = index === 1 ? "accent-arrow" : "wire-arrow";
          const strokeWidth = index === 1 ? 4 : 3;

          return (
            <Path
              key={material.label}
              points={[
                [materialRightX, material.y],
                [targetX, material.y],
                [targetX, shaderCodeBottomY + 10],
              ]}
              stroke={stroke}
              strokeWidth={strokeWidth}
              markerId={markerId}
            />
          );
        })}
      </SceneSvg>

      {renderInputAndOutputNodes()}

      <Box
        label=""
        x={OPEN_PSO.shell.x}
        y={OPEN_PSO.shell.y}
        width={OPEN_PSO.shell.width}
        height={OPEN_PSO.shell.height}
        fill={COLORS.surfaceAlt}
        stroke={COLORS.wireSoft}
        fontSize={34}
        fontWeight={650}
        radius={42}
      />

      <Caption label="PSO" x={0} y={-78} color={COLORS.ink} fontSize={30} fontWeight={700} />

      <Box
        label="ShaderCode"
        x={OPEN_PSO.shaderCode.x}
        y={OPEN_PSO.shaderCode.y}
        width={OPEN_PSO.shaderCode.width}
        height={OPEN_PSO.shaderCode.height}
        fill={accentSurface}
        stroke={accent}
        fontSize={28}
        fontWeight={650}
      />
      <Box
        label="Pipeline State"
        x={OPEN_PSO.pipelineState.x}
        y={OPEN_PSO.pipelineState.y}
        width={OPEN_PSO.pipelineState.width}
        height={OPEN_PSO.pipelineState.height}
        fontSize={26}
        fontWeight={620}
      />

      {OPEN_PSO.materials.map((material, index) => (
        <Box
          key={material.label}
          label={material.label}
          x={material.x}
          y={material.y}
          width={material.width}
          height={material.height}
          fill={index === 1 ? accentSurface : COLORS.surface}
          stroke={index === 1 ? accent : COLORS.wire}
          fontSize={22}
          fontWeight={index === 1 ? 650 : 600}
        />
      ))}

      <Pill
        label="Inline material path"
        x={-126}
        y={-132}
        width={228}
        fill={accentSurface}
        stroke={accent}
      />
    </>
  );
};

const SharedCodeScene: React.FC<{
  accent: string;
  accentSurface: string;
}> = ({accent, accentSurface}) => {
  const shaderCodeBottomY =
    OPEN_PSO.shaderCode.y + OPEN_PSO.shaderCode.height / 2 + 4;
  const shaderCodeLibTopY =
    OPEN_PSO.shaderCodeLib.y - OPEN_PSO.shaderCodeLib.height / 2 - 8;
  const shaderCodeLibLeftX =
    OPEN_PSO.shaderCodeLib.x - OPEN_PSO.shaderCodeLib.width / 2 - 8;

  return (
    <>
      <SceneSvg accent={accent}>
        <Path
          points={[
            [CORE.inputX + CORE.nodeWidth / 2 + 10, CORE.busY],
            [OPEN_PSO.shell.x - OPEN_PSO.shell.width / 2 - 10, CORE.busY],
          ]}
          markerId="wire-arrow"
        />
        <Path
          points={[
            [OPEN_PSO.shell.x + OPEN_PSO.shell.width / 2 + 10, CORE.busY],
            [CORE.outputX - CORE.nodeWidth / 2 - 10, CORE.busY],
          ]}
          markerId="wire-arrow"
        />

        {SHARED_MATERIAL_STACK.map((material) => (
          <Path
            key={`${material.label}-branch`}
            points={[
              [material.x + material.width / 2 + 4, material.y],
              [OPEN_PSO.sharedBusX, material.y],
            ]}
            strokeWidth={3}
          />
        ))}

        <Path
          points={[
            [OPEN_PSO.sharedBusX, SHARED_MATERIAL_STACK[0].y],
            [OPEN_PSO.sharedBusX, SHARED_MATERIAL_STACK[2].y],
          ]}
          strokeWidth={3}
        />
        <Path
          points={[
            [OPEN_PSO.sharedBusX, OPEN_PSO.shaderCodeLib.y],
            [shaderCodeLibLeftX, OPEN_PSO.shaderCodeLib.y],
          ]}
          strokeWidth={3}
          markerId="wire-arrow"
        />

        <Path
          points={[
            [OPEN_PSO.shaderCodeLib.x, shaderCodeLibTopY],
            [OPEN_PSO.shaderCodeLib.x, shaderCodeBottomY + 10],
          ]}
          stroke={accent}
          strokeWidth={4}
          markerId="accent-arrow"
        />
      </SceneSvg>

      {renderInputAndOutputNodes()}

      <Box
        label=""
        x={OPEN_PSO.shell.x}
        y={OPEN_PSO.shell.y}
        width={OPEN_PSO.shell.width}
        height={OPEN_PSO.shell.height}
        fill={COLORS.surfaceAlt}
        stroke={COLORS.wireSoft}
        fontSize={34}
        fontWeight={650}
        radius={42}
      />

      <Caption label="PSO" x={0} y={-78} color={COLORS.ink} fontSize={30} fontWeight={700} />

      <Box
        label="ShaderCode"
        x={OPEN_PSO.shaderCode.x}
        y={OPEN_PSO.shaderCode.y}
        width={OPEN_PSO.shaderCode.width}
        height={OPEN_PSO.shaderCode.height}
        fontSize={28}
        fontWeight={620}
      />
      <Box
        label="Pipeline State"
        x={OPEN_PSO.pipelineState.x}
        y={OPEN_PSO.pipelineState.y}
        width={OPEN_PSO.pipelineState.width}
        height={OPEN_PSO.pipelineState.height}
        fontSize={26}
        fontWeight={620}
      />

      {SHARED_MATERIAL_STACK.map((material) => (
        <Box
          key={material.label}
          label={material.label}
          x={material.x}
          y={material.y}
          width={material.width}
          height={material.height}
          fill={COLORS.surface}
          stroke={COLORS.wire}
          fontSize={22}
          fontWeight={600}
        />
      ))}

      <Box
        label="ShaderCodeLib"
        x={OPEN_PSO.shaderCodeLib.x}
        y={OPEN_PSO.shaderCodeLib.y}
        width={OPEN_PSO.shaderCodeLib.width}
        height={OPEN_PSO.shaderCodeLib.height}
        fill={accentSurface}
        stroke={accent}
        fontSize={24}
        fontWeight={650}
      />

      <Pill
        label="Shared ShaderCodeLib"
        x={-110}
        y={-132}
        width={246}
        fill={accentSurface}
        stroke={accent}
      />
    </>
  );
};

export const MyComposition: React.FC<RemotionWorkbenchProps> = ({
  variantId = "bus-clean",
}) => {
  const frame = useCurrentFrame();
  const sceneWindow = resolveRemotionSceneWindow(frame);
  const theme = VARIANT_THEME[variantId];

  return (
    <AbsoluteFill
      style={{
        background: "transparent",
        fontFamily: '"Avenir Next", "Helvetica Neue", sans-serif',
      }}
    >
      <div
        style={{
          position: "absolute",
          left: 0,
          top: 0,
          width: PANEL.width,
          height: PANEL.height,
          background: COLORS.panel,
          border: `2px solid ${COLORS.panelStroke}`,
          borderRadius: PANEL.radius,
          overflow: "hidden",
        }}
      >
        <SceneLayer stepId="base_formula" sceneWindow={sceneWindow}>
          <BaseFormulaScene
            accent={theme.accent}
            accentSurface={theme.accentSurface}
          />
        </SceneLayer>

        <SceneLayer stepId="opengl_state_machine" sceneWindow={sceneWindow}>
          <OpenGlScene accent={theme.accent} accentSoft={theme.accentSoft} />
        </SceneLayer>

        <SceneLayer stepId="vulkan_pso" sceneWindow={sceneWindow}>
          <VulkanScene
            accent={theme.accent}
            accentSoft={theme.accentSoft}
            accentSurface={theme.accentSurface}
          />
        </SceneLayer>

        <SceneLayer stepId="open_pso" sceneWindow={sceneWindow}>
          <OpenPsoScene
            accent={theme.accent}
            accentSurface={theme.accentSurface}
          />
        </SceneLayer>

        <SceneLayer stepId="inline_material" sceneWindow={sceneWindow}>
          <InlineMaterialScene
            accent={theme.accent}
            accentSurface={theme.accentSurface}
          />
        </SceneLayer>

        <SceneLayer stepId="shared_code" sceneWindow={sceneWindow}>
          <SharedCodeScene
            accent={theme.accent}
            accentSurface={theme.accentSurface}
          />
        </SceneLayer>
      </div>
    </AbsoluteFill>
  );
};
