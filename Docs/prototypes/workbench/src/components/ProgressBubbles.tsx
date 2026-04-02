import type {WorkbenchState} from "../state/useWorkbenchState";

type ProgressBubblesProps = {
  state: WorkbenchState;
};

export function ProgressBubbles({state}: ProgressBubblesProps) {
  return (
    <ol className="progress-bubbles" aria-label="Story progress">
      {state.steps.map((step, index) => {
        const stepIndex = state.steps.findIndex((entry) => entry.id === state.stepId);
        const bubbleState =
          index < stepIndex ? "past" : index === stepIndex ? "current" : "future";

        return (
          <li key={step.id} className="progress-bubble" data-state={bubbleState}>
            <span className="progress-step-index">Step {index + 1}</span>
            <span className="progress-step-label">{step.label}</span>
          </li>
        );
      })}
    </ol>
  );
}
