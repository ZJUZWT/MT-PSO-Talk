import {Player, Stage, Vector2} from "@motion-canvas/core";
import type {Project} from "@motion-canvas/core";
import project from "./project?project";
import {MOTION_CANVAS_PLAYER_CONFIG} from "./embed-config";
import {resolveMotionCanvasEmbedState} from "./embed-state";

const STAGE_SIZE = new Vector2(
  MOTION_CANVAS_PLAYER_CONFIG.width,
  MOTION_CANVAS_PLAYER_CONFIG.height,
);

async function bootstrap() {
  const container = document.getElementById("player-root");

  if (!container) {
    throw new Error("Missing #player-root container for Motion Canvas embed");
  }

  const initialSelection = resolveMotionCanvasEmbedState(window.location.search);

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
    initialSelection.initialFrame,
  );

  player.onRender.subscribe(async () => {
    await stage.render(player.playback.currentScene, player.playback.previousScene);
  });

  player.setVariables({variant: initialSelection.variantId});

  const applySelection = () => {
    const selection = resolveMotionCanvasEmbedState(window.location.search);
    player.setVariables({variant: selection.variantId});
    player.requestSeek(selection.initialFrame);
    player.requestRender();
  };

  window.addEventListener("popstate", applySelection);
}

void bootstrap();
