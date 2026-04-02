import {useState} from "react";
import {masterStoryboard} from "../../../shared/pso-storyboard";
import type {
  LibraryId,
  StoryStep,
  StoryStepId,
  VariantId,
} from "../../../shared/pso-workbench-types";
import {resolveLibraryState} from "../libraries/registry";
import type {LibraryDefinition, VariantOption} from "../libraries/types";

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
  supportedStepIds: StoryStepId[];
  libraryOptions: LibraryDefinition[];
  variantOptions: VariantOption[];
  activeLibrary: LibraryDefinition;
  activeVariant: VariantOption;
};

export function useWorkbenchState(): WorkbenchState {
  const [selection, setSelection] = useState(() =>
    resolveLibraryState({
      libraryId: "remotion",
      variantId: "bus-clean",
      stepId: "base_formula",
    }),
  );

  const currentStep =
    masterStoryboard.steps.find((step) => step.id === selection.stepId) ??
    masterStoryboard.steps[0];

  const setLibraryId = (libraryId: LibraryId) => {
    setSelection((current) =>
      resolveLibraryState({
        libraryId,
        variantId: current.variantId,
        stepId: current.stepId,
      }),
    );
  };

  const setVariantId = (variantId: VariantId) => {
    setSelection((current) =>
      resolveLibraryState({
        libraryId: current.libraryId,
        variantId,
        stepId: current.stepId,
      }),
    );
  };

  const setStepId = (stepId: StoryStepId) => {
    setSelection((current) =>
      resolveLibraryState({
        libraryId: current.libraryId,
        variantId: current.variantId,
        stepId,
      }),
    );
  };

  return {
    libraryId: selection.libraryId,
    setLibraryId,
    variantId: selection.variantId,
    setVariantId,
    stepId: selection.stepId,
    setStepId,
    aspectRatio: "16:9",
    steps: masterStoryboard.steps,
    currentStep,
    supportedStepIds: selection.supportedStepIds,
    libraryOptions: selection.libraryOptions,
    variantOptions: selection.variantOptions,
    activeLibrary: selection.activeLibrary,
    activeVariant: selection.activeVariant,
  };
}
