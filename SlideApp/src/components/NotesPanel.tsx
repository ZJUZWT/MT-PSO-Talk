import type {CSSProperties} from "react";
import type {WorkbenchState} from "../state/useWorkbenchState";

type NotesPanelProps = {
  state: WorkbenchState;
  transition: {
    direction: "forward" | "backward";
    outgoingStepId: WorkbenchState["stepId"];
  } | null;
};

function resolveCodeTone(
  focusColorKey: WorkbenchState["currentStep"]["focusColorKey"],
  line: string,
) {
  const normalizedLine = line.toLowerCase();

  if (focusColorKey === "opengl") {
    if (normalizedLine.includes("shader") || normalizedLine.includes("program")) {
      return "focus";
    }

    if (normalizedLine.includes("depth") || normalizedLine.includes("blend")) {
      return "support";
    }
  }

  if (focusColorKey === "vulkan") {
    if (
      normalizedLine.includes("pipeline") ||
      normalizedLine.includes("shader") ||
      normalizedLine.includes("raster") ||
      normalizedLine.includes("blend")
    ) {
      return "focus";
    }
  }

  return "default";
}

function LegendChip({label, color}: {label: string; color: string}) {
  return (
    <li
      className="notes-legend-chip"
      style={{"--legend-color": `var(${color})`} as CSSProperties}
    >
      <span className="notes-legend-swatch" aria-hidden="true" />
      <span>{label}</span>
    </li>
  );
}

function NotesCard({
  step,
  stepIndex,
  stepCount,
}: {
  step: WorkbenchState["currentStep"];
  stepIndex: number;
  stepCount: number;
}) {
  const codeLines = step.codeSample?.split("\n") ?? [];

  return (
    <article className="notes-card">
      <div className="notes-header">
        <div className="notes-kicker">{`Step ${stepIndex + 1} / ${stepCount}`}</div>
        <div className="notes-focus-pill">{step.focusTarget}</div>
      </div>

      <div className="notes-heading-block">
        <h1 className="notes-title">{step.label}</h1>
        <p className="notes-caption">{step.caption}</p>
      </div>

      {step.intro ? <p className="notes-intro">{step.intro}</p> : null}

      {step.manuscript ? (
        <section className="notes-section">
          <p className="notes-section-label">Manuscript</p>
          <p className="notes-section-copy">{step.manuscript}</p>
        </section>
      ) : null}

      <section className="notes-section">
        <p className="notes-section-label">Speaker cue</p>
        <p className="notes-section-copy">{step.notes}</p>
      </section>

      <section className="notes-meta-grid" aria-label="Step metadata">
        <div className="notes-meta-item">
          <p className="notes-section-label">Focus</p>
          <p className="notes-meta-copy">{step.focusTarget}</p>
        </div>
        <div className="notes-meta-item">
          <p className="notes-section-label">Timing</p>
          <p className="notes-meta-copy">{step.timingHint}</p>
        </div>
      </section>

      {codeLines.length > 0 ? (
        <section className="notes-code-panel" aria-label="Code sample">
          <div className="notes-code-header">
            <p className="notes-section-label">Code cue</p>
            {step.codeLegend?.length ? (
              <ul className="notes-legend" aria-label="Code legend">
                {step.codeLegend.map((entry) => (
                  <LegendChip
                    key={`${step.id}-${entry.label}`}
                    color={entry.color}
                    label={entry.label}
                  />
                ))}
              </ul>
            ) : null}
          </div>
          <pre className="notes-code-block">
            <code>
              {codeLines.map((line, index) => (
                <span
                  key={`${step.id}-${index}-${line}`}
                  className="notes-code-line"
                  data-tone={resolveCodeTone(step.focusColorKey, line)}
                >
                  {line}
                </span>
              ))}
            </code>
          </pre>
        </section>
      ) : null}
    </article>
  );
}

function NotesCardGhost() {
  return <div className="notes-card notes-card--ghost" aria-hidden="true" />;
}

export function NotesPanel({state, transition}: NotesPanelProps) {
  const outgoingStep = transition
    ? state.steps.find((step) => step.id === transition.outgoingStepId) ?? null
    : null;
  const stepCount = state.steps.length;
  const currentStackRole = transition ? "back" : "front";
  const outgoingStackRole = transition ? "front" : "back";
  const currentStepIndex = state.steps.findIndex(
    (step) => step.id === state.currentStep.id,
  );
  const outgoingStepIndex = outgoingStep
    ? state.steps.findIndex((step) => step.id === outgoingStep.id)
    : -1;

  return (
    <section
      className="notes-panel"
      aria-label="Speaker notes"
      data-motion-direction={transition?.direction ?? "idle"}
    >
      <div className="notes-stack">
        <div
          className="notes-card-layer notes-card-layer--current"
          data-motion-axis="vertical"
          data-motion-direction="idle"
          data-step-id={state.currentStep.id}
          data-stack-role={currentStackRole}
          data-fade="off"
        >
          <NotesCard
            step={state.currentStep}
            stepCount={stepCount}
            stepIndex={currentStepIndex}
          />
        </div>

        <div
          className="notes-card-layer notes-card-layer--outgoing"
          data-motion-axis="vertical"
          data-motion-direction={transition?.direction ?? "idle"}
          data-step-id={outgoingStep?.id}
          data-stack-role={outgoingStackRole}
          data-has-step={outgoingStep ? "true" : "false"}
          data-fade="off"
          aria-hidden="true"
        >
          {outgoingStep ? (
            <NotesCard
              step={outgoingStep}
              stepCount={stepCount}
              stepIndex={outgoingStepIndex}
            />
          ) : (
            <NotesCardGhost />
          )}
        </div>
      </div>
    </section>
  );
}
