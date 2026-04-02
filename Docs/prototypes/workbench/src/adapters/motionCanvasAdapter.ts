import type {StoryStepId, VariantId} from "../../../shared/pso-workbench-types";

type MotionCanvasWorkbenchSelection = {
  libraryId: "motion-canvas";
  variantId: VariantId;
  stepId: StoryStepId;
};

export function createMotionCanvasAdapter(
  baseUrl = "http://127.0.0.1:4175",
) {
  const normalizedBaseUrl = baseUrl.endsWith("/")
    ? baseUrl.slice(0, -1)
    : baseUrl;

  return {
    id: "motion-canvas" as const,
    getEmbedUrl(state: MotionCanvasWorkbenchSelection) {
      const query = new URLSearchParams({
        variant: state.variantId,
        step: state.stepId,
      });

      return `${normalizedBaseUrl}/embed.html?${query.toString()}`;
    },
  };
}
