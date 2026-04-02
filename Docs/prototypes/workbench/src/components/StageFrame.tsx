import {createManimAdapter} from "../adapters/manimAdapter";
import {createMotionCanvasAdapter} from "../adapters/motionCanvasAdapter";
import type {WorkbenchState} from "../state/useWorkbenchState";
import {createRemotionAdapter} from "../adapters/remotionAdapter";

type StageFrameProps = {
  state: WorkbenchState;
};

export function StageFrame({state}: StageFrameProps) {
  const manimAdapter = createManimAdapter();
  const remotionAdapter = createRemotionAdapter();
  const motionCanvasAdapter = createMotionCanvasAdapter("http://127.0.0.1:4175");
  const supportsRemotionStage = state.libraryId === "remotion";
  const supportsMotionCanvasStage = state.libraryId === "motion-canvas";

  return (
    <section className="stage-frame" aria-label="Animation stage">
      <div className="stage-meta">
        <div className="stage-chip">{state.activeLibrary.label}</div>
        <div className="stage-chip">{state.activeVariant.label}</div>
        <div className="stage-chip">{state.currentStep.label}</div>
      </div>

      <div
        className={`stage-card${
          supportsRemotionStage ? " stage-card--runtime" : ""
        }`}
      >
        {supportsRemotionStage ? (
          remotionAdapter.render({
            libraryId: "remotion",
            variantId: state.variantId,
            stepId: state.stepId,
          })
        ) : supportsMotionCanvasStage ? (
          <div className="stage-card-inner">
            <p className="notes-callout-label">Live bridge</p>
            <div className="stage-embed-frame">
              <iframe
                title="Motion Canvas runtime"
                src={motionCanvasAdapter.getEmbedUrl({
                  libraryId: "motion-canvas",
                  variantId: state.variantId,
                  stepId: state.stepId,
                })}
                className="stage-embed"
              />
            </div>
            <p className="stage-card-copy">
              The host can now target a Motion Canvas embed surface. Start the
              child runtime and the same storyboard state will flow through.
            </p>
          </div>
        ) : (
          <div className="stage-card-inner">
            <p className="notes-callout-label">Reference mode</p>
            <h2 className="stage-card-title">{state.currentStep.label}</h2>
            <p className="stage-card-copy">
              Manim currently resolves to a reference timestamp at{" "}
              {manimAdapter.resolveStep(state.stepId).timeInSeconds.toFixed(1)}s.
              The host keeps notes, progress, and active step stable while the
              final reference media is wired in.
            </p>
            <p className="notes-callout-copy">
              {manimAdapter.resolveStep(state.stepId).note}
            </p>
          </div>
        )}
      </div>
    </section>
  );
}
