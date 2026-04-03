import {useEffect, useRef} from "react";
import type {WorkbenchState} from "../state/useWorkbenchState";
import {RemotionStage} from "./RemotionStage";

type StageFrameProps = {
  state: WorkbenchState;
  motionDurationScale: number;
};

export function StageFrame({state, motionDurationScale}: StageFrameProps) {
  const previousSelectionRef = useRef<{
    stepId: WorkbenchState["stepId"];
    variantId: WorkbenchState["variantId"];
  } | null>(null);
  const previousStepId =
    previousSelectionRef.current?.variantId === state.variantId
      ? previousSelectionRef.current.stepId
      : null;

  useEffect(() => {
    previousSelectionRef.current = {
      stepId: state.stepId,
      variantId: state.variantId,
    };
  }, [state.stepId, state.variantId]);

  return (
    <section className="stage-frame" aria-label="Animation stage">
      <RemotionStage
        motionDurationScale={motionDurationScale}
        previousStepId={previousStepId}
        variantId={state.variantId}
        stepId={state.stepId}
      />
    </section>
  );
}
