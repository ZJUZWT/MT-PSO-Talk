import type {WorkbenchState} from "../state/useWorkbenchState";

type NotesPanelProps = {
  state: WorkbenchState;
};

export function NotesPanel({state}: NotesPanelProps) {
  return (
    <section className="notes-panel" aria-label="Speaker notes">
      <div className="notes-kicker">PSO Workbench</div>
      <div>
        <h1 className="notes-title">{state.currentStep.label}</h1>
      </div>
      <p className="notes-caption">{state.currentStep.caption}</p>
      <div className="notes-callout">
        <p className="notes-callout-label">Speaker note</p>
        <p className="notes-callout-copy">{state.currentStep.notes}</p>
      </div>
      <div className="notes-callout">
        <p className="notes-callout-label">Focus target</p>
        <p className="notes-callout-copy">{state.currentStep.focusTarget}</p>
      </div>
    </section>
  );
}
