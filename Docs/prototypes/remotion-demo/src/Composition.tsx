import React from "react";
import {
  AbsoluteFill,
  Easing,
  interpolate,
  useCurrentFrame,
} from "remotion";
import type {VariantId} from "../../shared/pso-workbench-types";
import type {RemotionWorkbenchProps} from "./embed";

const COLORS = {
  page: "#f6f2e9",
  pageAlt: "#f1ece2",
  panel: "#fbf8f2",
  panelStroke: "#dfd7ca",
  wire: "#55606c",
  ink: "#25313d",
  muted: "#6f786f",
  surface: "#fffdf8",
  emphasisSurface: "#f3e1d5",
  emphasis: "#ca6f45",
};

const VARIANT_PANEL_STYLE: Record<
  VariantId,
  {transform: string; emphasisSurface: string; emphasis: string}
> = {
  "bus-clean": {
    transform: "translate3d(0, 0, 0) scale(1)",
    emphasisSurface: COLORS.emphasisSurface,
    emphasis: COLORS.emphasis,
  },
  "bus-wide": {
    transform: "translate3d(-18px, 0, 0) scaleX(1.05)",
    emphasisSurface: "#efe6dc",
    emphasis: "#9e6f54",
  },
  "shared-focus": {
    transform: "translate3d(18px, 8px, 0) scale(1.03)",
    emphasisSurface: "#f0ddd0",
    emphasis: "#b85f3c",
  },
  reference: {
    transform: "translate3d(0, 0, 0) scale(1)",
    emphasisSurface: COLORS.emphasisSurface,
    emphasis: COLORS.emphasis,
  },
};

const PANEL = {
  width: 780,
  height: 572,
  left: 420,
  top: 74,
};

const ease = Easing.bezier(0.22, 1, 0.36, 1);

const ramp = (frame: number, start: number, duration: number) =>
  interpolate(frame, [start, start + duration], [0, 1], {
    easing: ease,
    extrapolateLeft: "clamp",
    extrapolateRight: "clamp",
  });

const mix = (from: number, to: number, progress: number) =>
  from + (to - from) * progress;

const centerToTopLeft = (x: number, y: number, width: number, height: number) => ({
  left: PANEL.width / 2 + x - width / 2,
  top: PANEL.height / 2 + y - height / 2,
});

type BoxProps = {
  label: string;
  x: number;
  y: number;
  width: number;
  height: number;
  opacity?: number;
  scale?: number;
  fill?: string;
  stroke?: string;
  fontSize?: number;
  fontWeight?: number;
  color?: string;
};

const Box: React.FC<BoxProps> = ({
  label,
  x,
  y,
  width,
  height,
  opacity = 1,
  scale = 1,
  fill = COLORS.surface,
  stroke = COLORS.wire,
  fontSize = 24,
  fontWeight = 600,
  color = COLORS.ink,
}) => {
  const position = centerToTopLeft(x, y, width, height);

  return (
    <div
      style={{
        position: "absolute",
        ...position,
        width,
        height,
        borderRadius: Math.min(height / 2, 28),
        border: `3px solid ${stroke}`,
        background: fill,
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        color,
        fontFamily: '"Avenir Next", "Helvetica Neue", sans-serif',
        fontSize,
        fontWeight,
        letterSpacing: "-0.03em",
        opacity,
        transform: `scale(${scale})`,
        transformOrigin: "center center",
        boxSizing: "border-box",
      }}
    >
      {label}
    </div>
  );
};

type ArrowProps = {
  id: string;
  points: Array<[number, number]>;
  progress: number;
  opacity?: number;
  stroke?: string;
  strokeWidth?: number;
  markerId: string;
};

const Arrow: React.FC<ArrowProps> = ({
  points,
  progress,
  opacity = 1,
  stroke = COLORS.wire,
  strokeWidth = 4,
  markerId,
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
      markerEnd={`url(#${markerId})`}
      pathLength={1}
      strokeDasharray="1"
      strokeDashoffset={1 - progress}
    />
  );
};

export const MyComposition: React.FC<RemotionWorkbenchProps> = ({
  variantId = "bus-clean",
}) => {
  const frame = useCurrentFrame();
  const variantPanelStyle = VARIANT_PANEL_STYLE[variantId];

  const appear = ramp(frame, 0, 18);
  const expand = ramp(frame, 20, 26);
  const inner = ramp(frame, 46, 14);
  const materials = ramp(frame, 62, 18);
  const share = ramp(frame, 88, 22);

  const fxWidth = mix(150, 388, expand);
  const fxHeight = mix(72, 224, expand);
  const fxLabelY = mix(0, -74, expand);
  const baseScale = mix(0.94, 1, appear);
  const contentScale = mix(0.94, 1, inner);
  const materialScale = mix(0.94, 1, materials);
  const shareScale = mix(0.94, 1, share);
  const inlineOpacity = materials * (1 - share * 0.82);

  return (
    <AbsoluteFill
      style={{
        background: `linear-gradient(135deg, ${COLORS.page} 0%, ${COLORS.pageAlt} 100%)`,
        fontFamily: '"Avenir Next", "Helvetica Neue", sans-serif',
      }}
    >
      <div
        style={{
          position: "absolute",
          left: PANEL.left,
          top: PANEL.top,
          width: PANEL.width,
          height: PANEL.height,
          background: COLORS.panel,
          border: `2px solid ${COLORS.panelStroke}`,
          borderRadius: 40,
          opacity: appear,
          transform: variantPanelStyle.transform,
          transformOrigin: "center center",
          boxShadow: "0 10px 30px rgba(80, 72, 56, 0.06)",
          overflow: "hidden",
        }}
      >
        <svg
          width={PANEL.width}
          height={PANEL.height}
          style={{ position: "absolute", inset: 0 }}
        >
          <defs>
            <marker
              id="wire-arrow"
              markerWidth="8"
              markerHeight="8"
              refX="6.4"
              refY="4"
              orient="auto"
              markerUnits="strokeWidth"
            >
              <path d="M0,0 L8,4 L0,8 z" fill={COLORS.wire} />
            </marker>
            <marker
              id="accent-arrow"
              markerWidth="8"
              markerHeight="8"
              refX="6.4"
              refY="4"
              orient="auto"
              markerUnits="strokeWidth"
            >
              <path d="M0,0 L8,4 L0,8 z" fill={variantPanelStyle.emphasis} />
            </marker>
          </defs>

          <Arrow
            id="a-to-f"
            points={[
              [-214, -92],
              [-fxWidth / 2 - 8, -92],
            ]}
            progress={appear}
            markerId="wire-arrow"
          />
          <Arrow
            id="f-to-b"
            points={[
              [fxWidth / 2 + 8, -92],
              [214, -92],
            ]}
            progress={appear}
            markerId="wire-arrow"
          />

          <Arrow
            id="inline-a"
            points={[
              [-228, 86],
              [-198, 86],
              [-198, 5],
              [-175, 5],
            ]}
            progress={materials}
            opacity={inlineOpacity}
            strokeWidth={3}
            markerId="wire-arrow"
          />
          <Arrow
            id="inline-b"
            points={[
              [-228, 156],
              [-190, 156],
              [-190, -4],
              [-175, -4],
            ]}
            progress={materials}
            opacity={inlineOpacity}
            strokeWidth={3}
            markerId="wire-arrow"
          />
          <Arrow
            id="inline-c"
            points={[
              [-228, 226],
              [-182, 226],
              [-182, -13],
              [-175, -13],
            ]}
            progress={materials}
            opacity={inlineOpacity}
            strokeWidth={3}
            markerId="wire-arrow"
          />

          <Arrow
            id="shared-a"
            points={[
              [-228, 86],
              [-196, 86],
              [-196, 156],
              [-181, 156],
            ]}
            progress={share}
            opacity={share}
            strokeWidth={3}
            markerId="wire-arrow"
          />
          <Arrow
            id="shared-b"
            points={[
              [-228, 156],
              [-181, 156],
            ]}
            progress={share}
            opacity={share}
            strokeWidth={3}
            markerId="wire-arrow"
          />
          <Arrow
            id="shared-c"
            points={[
              [-228, 226],
              [-196, 226],
              [-196, 156],
              [-181, 156],
            ]}
            progress={share}
            opacity={share}
            strokeWidth={3}
            markerId="wire-arrow"
          />

          <Arrow
            id="lib-to-shader"
            points={[
              [-5, 156],
              [-5, 40],
              [-58, 40],
              [-58, 15],
            ]}
            progress={share}
            opacity={share}
            stroke={COLORS.emphasis}
            strokeWidth={4}
            markerId="accent-arrow"
          />
        </svg>

        <Box
          label="A"
          x={-258}
          y={-92}
          width={84}
          height={62}
          opacity={appear}
          scale={baseScale}
          fontSize={34}
          fontWeight={650}
        />

        <div
          style={{
            position: "absolute",
            ...centerToTopLeft(0, -92, fxWidth, fxHeight),
            width: fxWidth,
            height: fxHeight,
            borderRadius: mix(30, 36, expand),
            border: `3px solid ${variantPanelStyle.emphasis}`,
            background: variantPanelStyle.emphasisSurface,
            boxSizing: "border-box",
            opacity: appear,
            transform: `scale(${baseScale})`,
            transformOrigin: "center center",
          }}
        >
          <div
            style={{
              position: "absolute",
              left: "50%",
              top: "50%",
              transform: `translate(-50%, calc(-50% + ${fxLabelY}px))`,
              color: COLORS.ink,
              fontSize: 34,
              fontWeight: 650,
              letterSpacing: "-0.03em",
            }}
          >
            f(x)
          </div>
        </div>

        <Box
          label="B"
          x={258}
          y={-92}
          width={84}
          height={62}
          opacity={appear}
          scale={baseScale}
          fontSize={34}
          fontWeight={650}
        />

        <Box
          label="ShaderCode"
          x={-96}
          y={-36}
          width={156}
          height={82}
          opacity={inner}
          scale={contentScale}
        />

        <Box
          label="Pipeline State"
          x={98}
          y={-36}
          width={162}
          height={82}
          opacity={inner}
          scale={contentScale}
          fontSize={23}
        />

        <Box
          label="Material A"
          x={-302}
          y={86}
          width={148}
          height={54}
          opacity={materials}
          scale={materialScale}
          fontSize={23}
        />
        <Box
          label="Material B"
          x={-302}
          y={156}
          width={148}
          height={54}
          opacity={materials}
          scale={materialScale}
          fontSize={23}
        />
        <Box
          label="Material C"
          x={-302}
          y={226}
          width={148}
          height={54}
          opacity={materials}
          scale={materialScale}
          fontSize={23}
        />

        <Box
          label="ShaderCodeLib"
          x={-90}
          y={156}
          width={170}
          height={70}
          opacity={share}
          scale={shareScale}
          fill={COLORS.surface}
          stroke={COLORS.emphasis}
          fontSize={22}
        />
      </div>

    </AbsoluteFill>
  );
};
