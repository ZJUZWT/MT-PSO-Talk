import type {
  LibraryId,
  StoryStep,
  StoryStepId,
  VariantId,
} from "../../../shared/pso-workbench-types";

export type VariantOption = {
  id: VariantId;
  label: string;
};

export type LibraryDefinition = {
  id: LibraryId;
  label: string;
  defaultVariantId: VariantId;
  supportedVariantIds: VariantId[];
  supportedStepIds: StoryStepId[];
};

export type ResolvedLibraryState = {
  libraryId: LibraryId;
  variantId: VariantId;
  stepId: StoryStepId;
  activeLibrary: LibraryDefinition;
  activeVariant: VariantOption;
  libraryOptions: LibraryDefinition[];
  variantOptions: VariantOption[];
  supportedStepIds: StoryStepId[];
  supportedSteps: StoryStep[];
};
