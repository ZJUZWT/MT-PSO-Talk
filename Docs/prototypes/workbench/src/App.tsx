import {ControlBar} from "./components/ControlBar";
import {NotesPanel} from "./components/NotesPanel";
import {ProgressBubbles} from "./components/ProgressBubbles";
import {StageFrame} from "./components/StageFrame";
import {useWorkbenchState} from "./state/useWorkbenchState";

export function App() {
  const state = useWorkbenchState();

  return (
    <div className="workbench-shell">
      <div className="workbench-glow workbench-glow-left" />
      <div className="workbench-glow workbench-glow-right" />
      <ControlBar state={state} />
      <main className="workbench-main">
        <NotesPanel state={state} />
        <StageFrame state={state} />
      </main>
      <ProgressBubbles state={state} />
    </div>
  );
}
