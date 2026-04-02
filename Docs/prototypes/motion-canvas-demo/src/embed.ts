import {Player, Stage, Vector2} from "@motion-canvas/core";
import type {Project} from "@motion-canvas/core";
import project from "./project";
import type {
  StoryStepId,
  VariantId,
} from "../../shared/pso-workbench-types";
import {
  MOTION_CANVAS_PLAYER_CONFIG,
  resolveMotionCanvasStepFrame,
} from "./embed-config";

const STAGE_SIZE = new Vector2(
  MOTION_CANVAS_PLAYER_CONFIG.width,
  MOTION_CANVAS_PLAYER_CONFIG.height,
);

function readSelectionFromLocation(): {
  stepId: StoryStepId;
  variantId: VariantId;
} {
  const params = new URLSearchParams(window.location.search);
  const requestedStep = params.get("step") as StoryStepId | null;
  const requestedVariant = params.get("variant") as VariantId | null;

  return {
    stepId: requestedStep ?? "base_formula",
    variantId: requestedVariant ?? "bus-clean",
  };
}

async function bootstrap() {
  const container = document.getElementById("player-root");

  if (!container) {
    throw new Error("Missing #player-root container for Motion Canvas embed");
  }

  const stage = new Stage();
  stage.configure({
    background: MOTION_CANVAS_PLAYER_CONFIG.background,
    colorSpace: "srgb",
    size: STAGE_SIZE,
    resolutionScale: 1,
  });

  stage.finalBuffer.setAttribute("aria-label", "Motion Canvas stage");
  container.replaceChildren(stage.finalBuffer);

  const player = new Player(
    project as Project,
    {
      fps: MOTION_CANVAS_PLAYER_CONFIG.fps,
      size: STAGE_SIZE,
      resolutionScale: 1,
      range: [0, Infinity],
      audioOffset: 0,
    },
    {
      loop: false,
      muted: true,
      paused: true,
      volume: 0,
    },
  );

  player.onRender.subscribe(async () => {
    await stage.render(player.playback.currentScene, player.playback.previousScene);
  });

  const applySelection = () => {
    const selection = readSelectionFromLocation();
    player.setVariables({variant: selection.variantId});
    player.requestSeek(resolveMotionCanvasStepFrame(selection.stepId));
    player.requestRender();
  };

  applySelection();
  window.addEventListener("popstate", applySelection);
}

void bootstrap();
