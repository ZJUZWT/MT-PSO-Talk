import {Line, Node, Rect, Txt, makeScene2D} from '@motion-canvas/2d';
import {all, createRef, createSignal, useScene, waitFor} from '@motion-canvas/core';
import type {VariantId} from '../../../shared/pso-workbench-types';

const COLORS = {
  page: '#f6f2e9',
  panel: '#fbf8f2',
  panelStroke: '#dfd7ca',
  wire: '#55606c',
  ink: '#25313d',
  surface: '#fffdf8',
  emphasisSurface: '#f3e1d5',
  emphasis: '#ca6f45',
};

const VARIANT_STAGE_STYLE: Record<
  VariantId,
  {
    stageOffsetX: number;
    stageOffsetY: number;
    stageScaleX: number;
    emphasisSurface: string;
    emphasis: string;
  }
> = {
  'bus-clean': {
    stageOffsetX: 0,
    stageOffsetY: 0,
    stageScaleX: 1,
    emphasisSurface: COLORS.emphasisSurface,
    emphasis: COLORS.emphasis,
  },
  'bus-wide': {
    stageOffsetX: -12,
    stageOffsetY: 0,
    stageScaleX: 1.03,
    emphasisSurface: '#efe6dc',
    emphasis: '#9e6f54',
  },
  'shared-focus': {
    stageOffsetX: 10,
    stageOffsetY: 8,
    stageScaleX: 1.02,
    emphasisSurface: '#f0ddd0',
    emphasis: '#b85f3c',
  },
  reference: {
    stageOffsetX: 0,
    stageOffsetY: 0,
    stageScaleX: 1,
    emphasisSurface: COLORS.emphasisSurface,
    emphasis: COLORS.emphasis,
  },
};

const FX_Y = -92;
const NODE_WIDTH = 84;
const NODE_HEIGHT = 62;
const FX_BASE_WIDTH = 150;
const FX_EXPANDED_WIDTH = 390;
const FX_BASE_HEIGHT = 72;
const FX_EXPANDED_HEIGHT = 224;
const SHADER_CODE_WIDTH = 168;
const SHADER_CODE_HEIGHT = 84;
const PIPELINE_WIDTH = 178;
const PIPELINE_HEIGHT = 84;
const MATERIAL_WIDTH = 152;
const MATERIAL_HEIGHT = 54;
const SHADER_LIB_WIDTH = 194;
const SHADER_LIB_HEIGHT = 72;
const MATERIAL_A_Y = 96;
const MATERIAL_B_Y = 168;
const MATERIAL_C_Y = 240;
const SHARED_BUS_X = -182;
const SHADER_LIB_BRANCH_Y = 188;
const INLINE_RAIL_A_X = -154;
const INLINE_RAIL_B_X = -142;
const INLINE_RAIL_C_X = -130;
const INLINE_PORT_A_Y = 0;
const INLINE_PORT_B_Y = -12;
const INLINE_PORT_C_Y = -24;
const LIB_VERTICAL_X = 78;

export default makeScene2D(function* (view) {
  const variant = useScene().variables.get<VariantId>('variant', 'bus-clean');
  const variantStyle = () => VARIANT_STAGE_STYLE[variant()];
  const stage = createRef<Rect>();
  const nodeA = createRef<Rect>();
  const nodeB = createRef<Rect>();
  const fxBox = createRef<Rect>();
  const fxLabel = createRef<Txt>();
  const shaderCode = createRef<Rect>();
  const pipelineState = createRef<Rect>();
  const materialA = createRef<Rect>();
  const materialB = createRef<Rect>();
  const materialC = createRef<Rect>();
  const shaderLib = createRef<Rect>();
  const lineAF = createRef<Line>();
  const lineFB = createRef<Line>();
  const inlineA = createRef<Line>();
  const inlineB = createRef<Line>();
  const inlineC = createRef<Line>();
  const sharedStubA = createRef<Line>();
  const sharedStubB = createRef<Line>();
  const sharedStubC = createRef<Line>();
  const sharedBus = createRef<Line>();
  const busToLib = createRef<Line>();
  const libToShader = createRef<Line>();

  const nodeAX = createSignal(-258);
  const nodeBX = createSignal(258);
  const fxX = createSignal(0);
  const fxWidth = createSignal(FX_BASE_WIDTH);
  const fxHeight = createSignal(FX_BASE_HEIGHT);
  const fxRadius = createSignal(30);
  const fxLabelY = createSignal(0);
  const shaderCodeX = createSignal(-96);
  const pipelineStateX = createSignal(98);
  const materialX = createSignal(-270);
  const shaderLibX = createSignal(-44);
  const shaderLibY = createSignal(SHADER_LIB_BRANCH_Y);

  const materialRight = () => materialX() + MATERIAL_WIDTH / 2;
  const shaderCodeLeft = () => shaderCodeX() - SHADER_CODE_WIDTH / 2;
  const shaderLibLeft = () => shaderLibX() - SHADER_LIB_WIDTH / 2;
  const shaderLibRight = () => shaderLibX() + SHADER_LIB_WIDTH / 2;
  const fxLeft = () => fxX() - fxWidth() / 2;
  const fxRight = () => fxX() + fxWidth() / 2;
  const fxBottom = () => FX_Y + fxHeight() / 2;

  view.add(
    <>
      <Rect width={1280} height={720} fill={COLORS.page} />
      <Rect
        ref={stage}
        x={() => 180 + variantStyle().stageOffsetX}
        y={() => variantStyle().stageOffsetY}
        width={780}
        height={572}
        scaleX={() => variantStyle().stageScaleX}
        radius={40}
        fill={COLORS.panel}
        stroke={COLORS.panelStroke}
        lineWidth={2}
        opacity={0}
      >
        <Node>
          <Line
            ref={lineAF}
            points={[
              () => [nodeAX() + NODE_WIDTH / 2, FX_Y],
              () => [fxLeft() - 14, FX_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={4}
            end={0}
            endArrow
            arrowSize={16}
          />
          <Line
            ref={lineFB}
            points={[
              () => [fxRight() + 14, FX_Y],
              () => [nodeBX() - NODE_WIDTH / 2, FX_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={4}
            end={0}
            endArrow
            arrowSize={16}
          />

          <Rect
            ref={nodeA}
            x={nodeAX}
            y={FX_Y}
            width={NODE_WIDTH}
            height={NODE_HEIGHT}
            radius={24}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'A'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={650}
              fontSize={34}
            />
          </Rect>

          <Rect
            ref={fxBox}
            x={fxX}
            y={FX_Y}
            width={fxWidth}
            height={fxHeight}
            radius={fxRadius}
            fill={() => variantStyle().emphasisSurface}
            stroke={() => variantStyle().emphasis}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              ref={fxLabel}
              y={fxLabelY}
              text={'f(x)'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={650}
              fontSize={34}
            />
          </Rect>

          <Rect
            ref={nodeB}
            x={nodeBX}
            y={FX_Y}
            width={NODE_WIDTH}
            height={NODE_HEIGHT}
            radius={24}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'B'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={650}
              fontSize={34}
            />
          </Rect>

          <Rect
            ref={shaderCode}
            x={shaderCodeX}
            y={-30}
            width={SHADER_CODE_WIDTH}
            height={SHADER_CODE_HEIGHT}
            radius={26}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'ShaderCode'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={24}
            />
          </Rect>

          <Rect
            ref={pipelineState}
            x={pipelineStateX}
            y={-30}
            width={PIPELINE_WIDTH}
            height={PIPELINE_HEIGHT}
            radius={26}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'Pipeline State'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={23}
            />
          </Rect>

          <Rect
            ref={materialA}
            x={materialX}
            y={MATERIAL_A_Y}
            width={MATERIAL_WIDTH}
            height={MATERIAL_HEIGHT}
            radius={22}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'Material A'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={23}
            />
          </Rect>

          <Rect
            ref={materialB}
            x={materialX}
            y={MATERIAL_B_Y}
            width={MATERIAL_WIDTH}
            height={MATERIAL_HEIGHT}
            radius={22}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'Material B'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={23}
            />
          </Rect>

          <Rect
            ref={materialC}
            x={materialX}
            y={MATERIAL_C_Y}
            width={MATERIAL_WIDTH}
            height={MATERIAL_HEIGHT}
            radius={22}
            fill={COLORS.surface}
            stroke={COLORS.wire}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'Material C'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={23}
            />
          </Rect>

          <Line
            ref={inlineA}
            points={[
              () => [materialRight(), MATERIAL_A_Y],
              [INLINE_RAIL_A_X, MATERIAL_A_Y],
              [INLINE_RAIL_A_X, INLINE_PORT_A_Y],
              () => [shaderCodeLeft() - 6, INLINE_PORT_A_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
            endArrow
            arrowSize={12}
          />
          <Line
            ref={inlineB}
            points={[
              () => [materialRight(), MATERIAL_B_Y],
              [INLINE_RAIL_B_X, MATERIAL_B_Y],
              [INLINE_RAIL_B_X, INLINE_PORT_B_Y],
              () => [shaderCodeLeft() - 6, INLINE_PORT_B_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
            endArrow
            arrowSize={12}
          />
          <Line
            ref={inlineC}
            points={[
              () => [materialRight(), MATERIAL_C_Y],
              [INLINE_RAIL_C_X, MATERIAL_C_Y],
              [INLINE_RAIL_C_X, INLINE_PORT_C_Y],
              () => [shaderCodeLeft() - 6, INLINE_PORT_C_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
            endArrow
            arrowSize={12}
          />

          <Rect
            ref={shaderLib}
            x={shaderLibX}
            y={shaderLibY}
            width={SHADER_LIB_WIDTH}
            height={SHADER_LIB_HEIGHT}
            radius={24}
            fill={COLORS.surface}
            stroke={COLORS.emphasis}
            lineWidth={3}
            opacity={0}
            scale={[0.94, 0.94]}
          >
            <Txt
              text={'ShaderCodeLib'}
              fill={COLORS.ink}
              fontFamily={'Avenir Next'}
              fontWeight={600}
              fontSize={22}
            />
          </Rect>

          <Line
            ref={sharedStubA}
            points={[
              () => [materialRight(), MATERIAL_A_Y],
              [SHARED_BUS_X, MATERIAL_A_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
          />
          <Line
            ref={sharedStubB}
            points={[
              () => [materialRight(), MATERIAL_B_Y],
              [SHARED_BUS_X, MATERIAL_B_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
          />
          <Line
            ref={sharedStubC}
            points={[
              () => [materialRight(), MATERIAL_C_Y],
              [SHARED_BUS_X, MATERIAL_C_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
          />
          <Line
            ref={sharedBus}
            points={[
              [SHARED_BUS_X, MATERIAL_A_Y],
              [SHARED_BUS_X, MATERIAL_C_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
          />
          <Line
            ref={busToLib}
            points={[
              [SHARED_BUS_X, SHADER_LIB_BRANCH_Y],
              () => [shaderLibLeft(), SHADER_LIB_BRANCH_Y],
            ]}
            stroke={COLORS.wire}
            lineWidth={3}
            end={0}
            endArrow
            arrowSize={12}
          />

          <Line
            ref={libToShader}
            points={[
              () => [shaderLibRight(), SHADER_LIB_BRANCH_Y],
              [LIB_VERTICAL_X, SHADER_LIB_BRANCH_Y],
              () => [LIB_VERTICAL_X, fxBottom() + 6],
            ]}
            stroke={COLORS.emphasis}
            lineWidth={4}
            end={0}
            endArrow
            arrowSize={15}
          />
        </Node>
      </Rect>
    </>,
  );

  yield* all(
    stage().opacity(1, 0.35),
    nodeA().opacity(1, 0.35),
    nodeA().scale([1, 1], 0.35),
    fxBox().opacity(1, 0.35),
    fxBox().scale([1, 1], 0.35),
    nodeB().opacity(1, 0.35),
    nodeB().scale([1, 1], 0.35),
    lineAF().end(1, 0.45),
    lineFB().end(1, 0.45),
  );

  yield* waitFor(0.12);

  yield* all(
    fxX(34, 0.56),
    fxWidth(FX_EXPANDED_WIDTH, 0.56),
    fxHeight(FX_EXPANDED_HEIGHT, 0.56),
    fxRadius(36, 0.56),
    fxLabelY(-74, 0.56),
    nodeAX(-270, 0.56),
    nodeBX(310, 0.56),
    shaderCodeX(-18, 0.56),
    pipelineStateX(152, 0.56),
  );

  yield* all(
    shaderCode().opacity(1, 0.34),
    shaderCode().scale([1, 1], 0.34),
    pipelineState().opacity(1, 0.34),
    pipelineState().scale([1, 1], 0.34),
  );

  yield* waitFor(0.08);

  yield* all(
    materialA().opacity(1, 0.3),
    materialA().scale([1, 1], 0.3),
    materialB().opacity(1, 0.3),
    materialB().scale([1, 1], 0.3),
    materialC().opacity(1, 0.3),
    materialC().scale([1, 1], 0.3),
    inlineA().end(1, 0.34),
    inlineB().end(1, 0.34),
    inlineC().end(1, 0.34),
  );

  yield* waitFor(0.14);

  yield* all(
    materialX(-306, 0.38),
    inlineA().opacity(0.04, 0.24),
    inlineB().opacity(0.04, 0.24),
    inlineC().opacity(0.04, 0.24),
    shaderLib().opacity(1, 0.34),
    shaderLib().scale([1, 1], 0.34),
    sharedStubA().end(1, 0.3),
    sharedStubB().end(1, 0.3),
    sharedStubC().end(1, 0.3),
    sharedBus().end(1, 0.3),
    busToLib().end(1, 0.32),
    libToShader().end(1, 0.38),
  );

  yield* waitFor(0.65);
});
