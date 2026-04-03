import {useState} from "react";
import {masterStoryboard} from "../storyboard-data/pso-storyboard";
import type {
  StoryStep,
  StoryStepId,
  VariantId,
} from "../storyboard-data/pso-workbench-types";

type VariantOption = {
  id: VariantId;
  label: string;
};

const VARIANT_OPTIONS: VariantOption[] = [
  {id: "bus-clean", label: "Bus Clean"},
  {id: "bus-wide", label: "Bus Wide"},
  {id: "shared-focus", label: "Shared Focus"},
];

export type WorkbenchState = {
  variantId: VariantId;
  setVariantId: (variantId: VariantId) => void;
  stepId: StoryStepId;
  setStepId: (stepId: StoryStepId) => void;
  goToPreviousStep: () => void;
  goToNextStep: () => void;
  aspectRatio: "16:9";
  steps: StoryStep[];
  currentStep: StoryStep;
  supportedStepIds: StoryStepId[];
  variantOptions: VariantOption[];
  activeVariant: VariantOption;
};

export function useWorkbenchState(): WorkbenchState {
  const [selection, setSelection] = useState<{
    variantId: VariantId;
    stepId: StoryStepId;
  }>({
    variantId: "bus-clean",
    stepId: "base_formula",
  });
  const supportedStepIds = masterStoryboard.steps.map((step) => step.id);

  const currentStep =
    masterStoryboard.steps.find((step) => step.id === selection.stepId) ??
    masterStoryboard.steps[0];
  const activeVariant =
    VARIANT_OPTIONS.find((option) => option.id === selection.variantId) ??
    VARIANT_OPTIONS[0];

  const setVariantId = (variantId: VariantId) => {
    setSelection((current) => ({
      ...current,
      variantId,
    }));
  };

  const setStepId = (stepId: StoryStepId) => {
    setSelection((current) => ({
      ...current,
      stepId,
    }));
  };

  const goToRelativeStep = (offset: -1 | 1) => {
    setSelection((current) => {
      const currentIndex = masterStoryboard.steps.findIndex(
        (step) => step.id === current.stepId,
      );
      const safeCurrentIndex = currentIndex === -1 ? 0 : currentIndex;
      const nextIndex = Math.max(
        0,
        Math.min(masterStoryboard.steps.length - 1, safeCurrentIndex + offset),
      );

      return {
        variantId: current.variantId,
        stepId: masterStoryboard.steps[nextIndex]?.id ?? current.stepId,
      };
    });
  };

  return {
    variantId: selection.variantId,
    setVariantId,
    stepId: selection.stepId,
    setStepId,
    goToPreviousStep: () => {
      goToRelativeStep(-1);
    },
    goToNextStep: () => {
      goToRelativeStep(1);
    },
    aspectRatio: "16:9",
    steps: masterStoryboard.steps,
    currentStep,
    supportedStepIds,
    variantOptions: VARIANT_OPTIONS,
    activeVariant,
  };
}
