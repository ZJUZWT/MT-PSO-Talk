import {Player} from "@remotion/player";
import type {StoryStepId, VariantId} from "../../../shared/pso-workbench-types";
import {MyComposition} from "../../../remotion-demo/src/Composition";
import {
  REMOTION_PLAYER_CONFIG,
  resolveRemotionStepFrame,
  type RemotionWorkbenchProps,
} from "../../../remotion-demo/src/embed";

type RemotionWorkbenchSelection = {
  libraryId: "remotion";
  variantId: VariantId;
  stepId: StoryStepId;
};

export function createRemotionAdapter() {
  const resolvePlayerState = (state: RemotionWorkbenchSelection) => {
    return {
      inputProps: {
        variantId: state.variantId,
      } satisfies RemotionWorkbenchProps,
      initialFrame: resolveRemotionStepFrame(state.stepId),
    };
  };

  return {
    id: "remotion" as const,
    resolvePlayerState,
    render(state: RemotionWorkbenchSelection) {
      const playerState = resolvePlayerState(state);

      return (
        <div className="stage-runtime">
          <Player
            key={`${state.variantId}:${state.stepId}`}
            component={MyComposition}
            compositionWidth={REMOTION_PLAYER_CONFIG.compositionWidth}
            compositionHeight={REMOTION_PLAYER_CONFIG.compositionHeight}
            durationInFrames={REMOTION_PLAYER_CONFIG.durationInFrames}
            fps={REMOTION_PLAYER_CONFIG.fps}
            inputProps={playerState.inputProps}
            initialFrame={playerState.initialFrame}
            acknowledgeRemotionLicense
            controls={false}
            clickToPlay={false}
            showPosterWhenUnplayed={false}
            style={{width: "100%", height: "100%"}}
          />
        </div>
      );
    },
  };
}
