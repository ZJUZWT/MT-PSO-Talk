import {masterStoryboard} from "../../../shared/pso-storyboard";
import type {
  LibraryId,
  StoryStepId,
  VariantId,
} from "../../../shared/pso-workbench-types";
import type {
  LibraryDefinition,
  ResolvedLibraryState,
  VariantOption,
} from "./types";

const variantCatalog: Record<VariantId, VariantOption> = {
  "bus-clean": {id: "bus-clean", label: "Bus Clean"},
  "bus-wide": {id: "bus-wide", label: "Bus Wide"},
  "shared-focus": {id: "shared-focus", label: "Shared Focus"},
  reference: {id: "reference", label: "Reference"},
};

export const libraryRegistry: Record<LibraryId, LibraryDefinition> = {
  remotion: {
    id: "remotion",
    label: "Remotion",
    defaultVariantId: "bus-clean",
    supportedVariantIds: ["bus-clean", "bus-wide", "shared-focus"],
    supportedStepIds: masterStoryboard.steps.map((step) => step.id),
  },
  "motion-canvas": {
    id: "motion-canvas",
    label: "Motion Canvas",
    defaultVariantId: "bus-clean",
    supportedVariantIds: ["bus-clean", "bus-wide", "shared-focus"],
    supportedStepIds: masterStoryboard.steps.map((step) => step.id),
  },
  manim: {
    id: "manim",
    label: "Manim",
    defaultVariantId: "reference",
    supportedVariantIds: ["reference"],
    supportedStepIds: ["base_formula", "shared_code"],
  },
};

export const libraryOptions = Object.values(libraryRegistry);

function resolveNearestStepId(
  requestedStepId: StoryStepId,
  supportedStepIds: StoryStepId[],
): StoryStepId {
  if (supportedStepIds.includes(requestedStepId)) {
    return requestedStepId;
  }

  const requestedIndex = masterStoryboard.steps.findIndex(
    (step) => step.id === requestedStepId,
  );

  if (requestedIndex === -1 || supportedStepIds.length === 0) {
    return masterStoryboard.steps[0].id;
  }

  return supportedStepIds.reduce((bestStepId, candidateStepId) => {
    const bestIndex = masterStoryboard.steps.findIndex(
      (step) => step.id === bestStepId,
    );
    const candidateIndex = masterStoryboard.steps.findIndex(
      (step) => step.id === candidateStepId,
    );

    const bestDistance = Math.abs(bestIndex - requestedIndex);
    const candidateDistance = Math.abs(candidateIndex - requestedIndex);

    if (candidateDistance < bestDistance) {
      return candidateStepId;
    }

    return bestStepId;
  }, supportedStepIds[0]);
}

export function resolveLibraryState(input: {
  libraryId: LibraryId;
  variantId: VariantId;
  stepId: StoryStepId;
}): ResolvedLibraryState {
  const activeLibrary = libraryRegistry[input.libraryId];
  const variantId = activeLibrary.supportedVariantIds.includes(input.variantId)
    ? input.variantId
    : activeLibrary.defaultVariantId;

  const stepId = resolveNearestStepId(input.stepId, activeLibrary.supportedStepIds);
  const variantOptions = activeLibrary.supportedVariantIds.map(
    (entry) => variantCatalog[entry],
  );
  const supportedSteps = masterStoryboard.steps.filter((step) =>
    activeLibrary.supportedStepIds.includes(step.id),
  );

  return {
    libraryId: activeLibrary.id,
    variantId,
    stepId,
    activeLibrary,
    activeVariant: variantCatalog[variantId],
    libraryOptions,
    variantOptions,
    supportedStepIds: activeLibrary.supportedStepIds,
    supportedSteps,
  };
}
