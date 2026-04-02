import type {WorkbenchState} from "../state/useWorkbenchState";
import {createRemotionAdapter} from "../adapters/remotionAdapter";

type StageFrameProps = {
  state: WorkbenchState;
};

export function StageFrame({state}: StageFrameProps) {
  const remotionAdapter = createRemotionAdapter();
  const supportsLiveStage = state.libraryId === "remotion";

  return (
    <section className="stage-frame" aria-label="Animation stage">
      <div className="stage-meta">
        <div className="stage-chip">{state.activeLibrary.label}</div>
        <div className="stage-chip">{state.activeVariant.label}</div>
        <div className="stage-chip">{state.currentStep.label}</div>
      </div>

      <div className={`stage-card${supportsLiveStage ? " stage-card--runtime" : ""}`}>
        {supportsLiveStage ? (
          remotionAdapter.render({
            libraryId: "remotion",
            variantId: state.variantId,
            stepId: state.stepId,
          })
        ) : (
          <div className="stage-card-inner">
            <p className="notes-callout-label">Stage placeholder</p>
            <h2 className="stage-card-title">{state.currentStep.label}</h2>
            <p className="stage-card-copy">
              The unified host shell is live. Next we will replace this stage
              card with runtime-backed surfaces for each animation library.
            </p>
          </div>
        )}
      </div>
    </section>
  );
}
