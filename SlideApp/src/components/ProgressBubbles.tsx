import type {CSSProperties} from "react";
import type {WorkbenchState} from "../state/useWorkbenchState";

type ProgressBubblesProps = {
  state: WorkbenchState;
  transition: {
    direction: "forward" | "backward";
    outgoingStepId: WorkbenchState["stepId"];
  } | null;
};

function resolveShellWidth(label: string, sizeMode: string) {
  if (sizeMode === "expanded" || sizeMode === "expanding") {
    return Math.max(176, Math.min(292, 116 + label.length * 8));
  }

  return 28;
}

function resolveBubbleHeight(sizeMode: string) {
  return sizeMode === "compact" || sizeMode === "collapsing" ? 28 : 74;
}

export function ProgressBubbles({state, transition}: ProgressBubblesProps) {
  const stepIndex = state.steps.findIndex((entry) => entry.id === state.stepId);

  return (
    <div className="progress-rail-window">
      <ol
        className="progress-bubbles"
        aria-label="Story progress"
        data-cross-align="centered"
      >
        {state.steps.map((step, index) => {
          const bubbleState =
            index < stepIndex ? "past" : index === stepIndex ? "current" : "future";
          const isCurrent = step.id === state.stepId;
          const isOutgoing = transition?.outgoingStepId === step.id;
          const sizeMode = isCurrent
            ? transition
              ? "expanding"
              : "expanded"
            : isOutgoing
              ? "collapsing"
              : "compact";
          const shellWidth = resolveShellWidth(step.label, sizeMode);
          const isCompact = sizeMode === "compact";
          const isCompactVisual = sizeMode === "compact" || sizeMode === "collapsing";
          const bubbleHeight = resolveBubbleHeight(sizeMode);
          const shapeMode = isCompactVisual ? "dot" : "pill";

          return (
            <li
              key={step.id}
              className="progress-step-shell"
              data-step-id={step.id}
              data-state={bubbleState}
              data-layout={isCompactVisual ? "compact" : "inline"}
              data-size-mode={sizeMode}
              style={
                {
                  "--step-shell-width": `${shellWidth}px`,
                } as CSSProperties
              }
            >
              <button
                type="button"
                className="progress-bubble"
                data-state={bubbleState}
                data-size-mode={sizeMode}
                data-shape-mode={shapeMode}
                data-compact={isCompactVisual ? "true" : "false"}
                data-single-line={isCompact ? "false" : "true"}
                style={
                  {
                    "--bubble-block-size": `${bubbleHeight}px`,
                  } as CSSProperties
                }
                aria-current={isCurrent ? "step" : undefined}
                aria-label={`Go to step ${index + 1}: ${step.label}`}
                onClick={() => {
                  state.setStepId(step.id);
                }}
              >
                <span className="progress-bubble-dot" aria-hidden="true" />
                <span className="progress-bubble-copy">
                  <span className="progress-step-index">Step {index + 1}</span>
                  <span className="progress-step-label">{step.label}</span>
                </span>
              </button>
            </li>
          );
        })}
      </ol>
    </div>
  );
}
