import type {CSSProperties} from "react";
import {useEffect, useLayoutEffect, useMemo, useRef, useState} from "react";
import {ControlBar} from "./components/ControlBar";
import {NotesPanel} from "./components/NotesPanel";
import {ProgressBubbles} from "./components/ProgressBubbles";
import {StageFrame} from "./components/StageFrame";
import {useWorkbenchState} from "./state/useWorkbenchState";

const MOTION_PRESETS = [
  {id: "quarter", label: "0.25x", durationScale: 4},
  {id: "half", label: "0.5x", durationScale: 2},
  {id: "normal", label: "1x", durationScale: 1},
  {id: "fast", label: "1.5x", durationScale: 0.72},
] as const;

type MotionPresetId = (typeof MOTION_PRESETS)[number]["id"];
const NOTES_TRANSITION_BASE_MS = 300;
const RAIL_FRAME_HEIGHT_PX = 104;
const RAIL_SPEED_FACTOR = 0.75;

function shouldIgnoreKeyboardNavigation(target: EventTarget | null): boolean {
  if (!(target instanceof HTMLElement)) {
    return false;
  }

  return Boolean(
    target.closest(
      'input, textarea, select, button, [contenteditable="true"], [role="textbox"]',
    ),
  );
}

export function App() {
  const state = useWorkbenchState();
  const [controlsCollapsed, setControlsCollapsed] = useState(true);
  const [motionPresetId, setMotionPresetId] = useState<MotionPresetId>("normal");
  const [stepTransition, setStepTransition] = useState<{
    direction: "forward" | "backward";
    outgoingStepId: typeof state.stepId;
  } | null>(null);
  const settledStepIdRef = useRef(state.stepId);
  const latestTargetStepIdRef = useRef(state.stepId);
  const motionPreset = useMemo(
    () =>
      MOTION_PRESETS.find((preset) => preset.id === motionPresetId) ??
      MOTION_PRESETS[2],
    [motionPresetId],
  );
  const railDurationScale = 1 / RAIL_SPEED_FACTOR;
  const stepTransitionMs = Math.round(
    NOTES_TRANSITION_BASE_MS * motionPreset.durationScale,
  );

  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (
        event.defaultPrevented ||
        event.altKey ||
        event.ctrlKey ||
        event.metaKey ||
        shouldIgnoreKeyboardNavigation(event.target ?? document.activeElement)
      ) {
        return;
      }

      if (event.key === "[" || event.key === "]" || event.key === "0") {
        event.preventDefault();
        setMotionPresetId((current) => {
          if (event.key === "0") {
            return "normal";
          }

          const currentIndex = MOTION_PRESETS.findIndex(
            (preset) => preset.id === current,
          );
          const safeIndex = currentIndex === -1 ? 2 : currentIndex;
          const nextIndex =
            event.key === "["
              ? Math.max(0, safeIndex - 1)
              : Math.min(MOTION_PRESETS.length - 1, safeIndex + 1);

          return MOTION_PRESETS[nextIndex]?.id ?? current;
        });
        return;
      }

      if (event.key === "ArrowLeft" || event.key === "ArrowUp") {
        event.preventDefault();
        state.goToPreviousStep();
      }

      if (event.key === "ArrowRight" || event.key === "ArrowDown") {
        event.preventDefault();
        state.goToNextStep();
      }
    };

    document.addEventListener("keydown", handleKeyDown);

    return () => {
      document.removeEventListener("keydown", handleKeyDown);
    };
  }, [state]);

  useLayoutEffect(() => {
    latestTargetStepIdRef.current = state.stepId;
    const settledStepId = settledStepIdRef.current;

    if (settledStepId === state.stepId) {
      setStepTransition(null);
      return;
    }

    const previousIndex = state.steps.findIndex((step) => step.id === settledStepId);
    const currentIndex = state.steps.findIndex((step) => step.id === state.stepId);

    setStepTransition({
      direction: currentIndex >= previousIndex ? "forward" : "backward",
      outgoingStepId: settledStepId,
    });

    const timeoutId = window.setTimeout(() => {
      settledStepIdRef.current = latestTargetStepIdRef.current;
      setStepTransition(null);
    }, stepTransitionMs + 36);

    return () => {
      window.clearTimeout(timeoutId);
    };
  }, [state.stepId, state.steps, stepTransitionMs]);

  return (
    <div
      className="workbench-shell"
      data-motion-preset={motionPreset.id}
      style={
        {
          "--motion-duration-scale": motionPreset.durationScale,
          "--rail-frame-height": `${RAIL_FRAME_HEIGHT_PX}px`,
          "--rail-speed-factor": `${RAIL_SPEED_FACTOR}`,
          "--rail-duration-scale": `${railDurationScale}`,
        } as CSSProperties
      }
    >
      <div className="workbench-glow workbench-glow-left" />
      <div className="workbench-glow workbench-glow-right" />
      <ControlBar
        state={state}
        collapsed={controlsCollapsed}
        motionPresetId={motionPreset.id}
        motionOptions={MOTION_PRESETS.map(({id, label}) => ({id, label}))}
        onMotionPresetChange={(nextPresetId) => {
          setMotionPresetId(nextPresetId as MotionPresetId);
        }}
        onToggleCollapsed={() => {
          setControlsCollapsed((current) => !current);
        }}
      />
      <main className="workbench-main">
        <NotesPanel state={state} transition={stepTransition} />
        <StageFrame
          state={state}
          motionDurationScale={motionPreset.durationScale}
        />
      </main>
      <ProgressBubbles state={state} transition={stepTransition} />
    </div>
  );
}
