import {useState} from "react";
import {
  libraryOptions,
  masterStoryboard,
  variantOptions,
} from "../../../shared/pso-storyboard";
import type {
  LibraryId,
  StoryStep,
  StoryStepId,
  VariantId,
} from "../../../shared/pso-workbench-types";

export type WorkbenchState = {
  libraryId: LibraryId;
  setLibraryId: (libraryId: LibraryId) => void;
  variantId: VariantId;
  setVariantId: (variantId: VariantId) => void;
  stepId: StoryStepId;
  setStepId: (stepId: StoryStepId) => void;
  aspectRatio: "16:9";
  steps: StoryStep[];
  currentStep: StoryStep;
  libraryOptions: typeof libraryOptions;
  variantOptions: typeof variantOptions;
  activeLibrary: (typeof libraryOptions)[number];
  activeVariant: (typeof variantOptions)[number];
};

export function useWorkbenchState(): WorkbenchState {
  const [libraryId, setLibraryId] = useState<LibraryId>("remotion");
  const [variantId, setVariantId] = useState<VariantId>("bus-clean");
  const [stepId, setStepId] = useState<StoryStepId>("base_formula");

  const currentStep =
    masterStoryboard.steps.find((step) => step.id === stepId) ??
    masterStoryboard.steps[0];

  const activeLibrary =
    libraryOptions.find((option) => option.id === libraryId) ?? libraryOptions[0];

  const activeVariant =
    variantOptions.find((option) => option.id === variantId) ?? variantOptions[0];

  return {
    libraryId,
    setLibraryId,
    variantId,
    setVariantId,
    stepId,
    setStepId,
    aspectRatio: "16:9",
    steps: masterStoryboard.steps,
    currentStep,
    libraryOptions,
    variantOptions,
    activeLibrary,
    activeVariant,
  };
}
