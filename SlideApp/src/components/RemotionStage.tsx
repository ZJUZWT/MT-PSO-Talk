import {Player} from "@remotion/player";
import type {PlayerRef} from "@remotion/player";
import {useEffect, useMemo, useRef, useState} from "react";
import type {
  StoryStepId,
  VariantId,
} from "../storyboard-data/pso-workbench-types";
import {resolvePlaybackFrame} from "../storyboard-data/playbackTimeline";
import {MyComposition} from "../remotion/Composition";
import {REMOTION_PLAYER_CONFIG} from "../remotion/embed";
import {createRemotionAdapter} from "../adapters/remotionAdapter";
import {createPlaybackPlan} from "../storyboard/playbackPlan";

type RemotionStageProps = {
  motionDurationScale: number;
  previousStepId: StoryStepId | null;
  stepId: StoryStepId;
  variantId: VariantId;
};

export function RemotionStage({
  motionDurationScale,
  previousStepId,
  stepId,
  variantId,
}: RemotionStageProps) {
  const adapter = useMemo(() => createRemotionAdapter(), []);
  const playerRef = useRef<PlayerRef>(null);
  const animationHandleRef = useRef<number | null>(null);
  const lastAppliedFrameRef = useRef<number>(adapter.getFrameForStep(stepId));
  const [currentFrame, setCurrentFrame] = useState(
    adapter.getFrameForStep(stepId),
  );
  const [isAnimating, setIsAnimating] = useState(false);
  const plan = useMemo(
    () =>
      createPlaybackPlan({
        previousStepId,
        stepId,
        resolveFrame: adapter.getFrameForStep,
      }),
    [adapter, previousStepId, stepId],
  );
  const effectiveDurationMs = plan.shouldAnimate
    ? Math.max(120, Math.round(plan.durationMs * motionDurationScale))
    : 0;

  useEffect(() => {
    const player = playerRef.current;

    if (!player) {
      return;
    }

    const stopAnimation = () => {
      if (animationHandleRef.current !== null) {
        cancelAnimationFrame(animationHandleRef.current);
        animationHandleRef.current = null;
      }
    };

    const applyFrame = (frame: number) => {
      if (lastAppliedFrameRef.current === frame) {
        return;
      }

      player.seekTo(frame);
      lastAppliedFrameRef.current = frame;
      setCurrentFrame(frame);
    };

    stopAnimation();
    applyFrame(plan.fromFrame);
    player.pause();
    setCurrentFrame(plan.fromFrame);
    setIsAnimating(plan.shouldAnimate);

    if (!plan.shouldAnimate) {
      applyFrame(plan.toFrame);
      setIsAnimating(false);
      return stopAnimation;
    }

    const startedAt = performance.now();

    const tick = (now: number) => {
      const elapsedMs = now - startedAt;
      const progress =
        effectiveDurationMs <= 0 ? 1 : Math.min(1, elapsedMs / effectiveDurationMs);
      const nextFrame = resolvePlaybackFrame({
        fromFrame: plan.fromFrame,
        toFrame: plan.toFrame,
        durationMs: effectiveDurationMs,
        elapsedMs,
      });

      applyFrame(nextFrame);

      if (progress < 1) {
        animationHandleRef.current = requestAnimationFrame(tick);
        return;
      }

      animationHandleRef.current = null;
      applyFrame(plan.toFrame);
      player.pause();
      setIsAnimating(false);
    };

    animationHandleRef.current = requestAnimationFrame(tick);

    return stopAnimation;
  }, [
    effectiveDurationMs,
    plan.fromFrame,
    plan.shouldAnimate,
    plan.toFrame,
    variantId,
  ]);

  return (
    <div
      className="stage-runtime"
      data-current-frame={currentFrame}
      data-animating={isAnimating ? "true" : "false"}
      data-motion-scale={motionDurationScale}
    >
      <Player
        ref={playerRef}
        key={variantId}
        component={MyComposition}
        compositionWidth={REMOTION_PLAYER_CONFIG.compositionWidth}
        compositionHeight={REMOTION_PLAYER_CONFIG.compositionHeight}
        durationInFrames={REMOTION_PLAYER_CONFIG.durationInFrames}
        fps={REMOTION_PLAYER_CONFIG.fps}
        inputProps={adapter.createInputProps(variantId)}
        initialFrame={plan.fromFrame}
        acknowledgeRemotionLicense
        controls={false}
        clickToPlay={false}
        moveToBeginningWhenEnded={false}
        showPosterWhenUnplayed={false}
        style={{width: "100%", height: "100%"}}
      />
    </div>
  );
}
