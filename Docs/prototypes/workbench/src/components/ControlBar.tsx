import type {ChangeEvent} from "react";
import type {WorkbenchState} from "../state/useWorkbenchState";

type ControlBarProps = {
  state: WorkbenchState;
};

export function ControlBar({state}: ControlBarProps) {
  const handleLibraryChange = (event: ChangeEvent<HTMLSelectElement>) => {
    state.setLibraryId(event.target.value as WorkbenchState["libraryId"]);
  };

  const handleVariantChange = (event: ChangeEvent<HTMLSelectElement>) => {
    state.setVariantId(event.target.value as WorkbenchState["variantId"]);
  };

  const handleStepChange = (event: ChangeEvent<HTMLSelectElement>) => {
    state.setStepId(event.target.value as WorkbenchState["stepId"]);
  };

  return (
    <section className="control-bar" aria-label="Workbench controls">
      <div className="control-card">
        <label htmlFor="library">Library</label>
        <select id="library" value={state.libraryId} onChange={handleLibraryChange}>
          {state.libraryOptions.map((option) => (
            <option key={option.id} value={option.id}>
              {option.label}
            </option>
          ))}
        </select>
      </div>

      <div className="control-card">
        <label htmlFor="variant">Variant</label>
        <select id="variant" value={state.variantId} onChange={handleVariantChange}>
          {state.variantOptions.map((option) => (
            <option key={option.id} value={option.id}>
              {option.label}
            </option>
          ))}
        </select>
      </div>

      <div className="control-card">
        <label htmlFor="step">Step</label>
        <select id="step" value={state.stepId} onChange={handleStepChange}>
          {state.steps.map((step) => (
            <option
              key={step.id}
              value={step.id}
              disabled={!state.supportedStepIds.includes(step.id)}
            >
              {step.label}
            </option>
          ))}
        </select>
      </div>

      <div className="control-card">
        <label htmlFor="aspect">Aspect</label>
        <select id="aspect" value={state.aspectRatio} onChange={() => undefined}>
          <option value="16:9">16:9</option>
        </select>
      </div>
    </section>
  );
}
