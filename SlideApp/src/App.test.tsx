import {
  cleanup,
  fireEvent,
  render,
  screen,
  waitFor,
} from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import {afterEach, describe, expect, it, vi} from "vitest";
import {App} from "./App";

afterEach(() => {
  cleanup();
});

describe("App", () => {
  it("shows the current step with controls collapsed by default", () => {
    render(<App />);

    expect(
      screen.getByRole("heading", {name: "A -> f(x) -> B", level: 1}),
    ).toBeInTheDocument();
    expect(
      screen.getByText(
        "Input moves through a single function so we can focus on what PSO will later refine.",
      ),
    ).toBeInTheDocument();
    expect(screen.getByRole("button", {name: "Show controls"})).toBeInTheDocument();
    expect(screen.getAllByRole("listitem").length).toBeGreaterThanOrEqual(4);
    expect(screen.queryByLabelText("Library")).not.toBeInTheDocument();
    expect(screen.queryByText("Remotion")).not.toBeInTheDocument();
    expect(screen.queryByLabelText("Variant")).not.toBeInTheDocument();
    expect(screen.queryByLabelText("Motion")).not.toBeInTheDocument();
  });

  it("renders browser-neutral select shells for all four controls", () => {
    render(<App />);
    fireEvent.click(screen.getByRole("button", {name: "Show controls"}));

    expect(document.querySelectorAll(".control-select-shell")).toHaveLength(4);
    expect(document.querySelectorAll(".control-select-icon")).toHaveLength(4);
  });

  it("dedicates the right panel to the runtime canvas without an extra inner card", () => {
    render(<App />);

    expect(document.querySelector(".stage-card")).not.toBeInTheDocument();
    expect(
      screen.getByLabelText("Animation stage").querySelector(".stage-runtime"),
    ).toBeInTheDocument();
  });

  it("keeps the editing controls focused on variant, step, aspect, and motion", async () => {
    const user = userEvent.setup();

    render(<App />);
    await user.click(screen.getByRole("button", {name: "Show controls"}));

    await user.selectOptions(screen.getByLabelText("Variant"), "shared-focus");
    await user.selectOptions(screen.getByLabelText("Step"), "shared_code");
    await user.selectOptions(screen.getByLabelText("Motion"), "half");

    expect(screen.getByLabelText("Variant")).toHaveValue("shared-focus");
    expect(screen.getByLabelText("Step")).toHaveValue("shared_code");
    expect(screen.getByLabelText("Aspect")).toHaveValue("16:9");
    expect(screen.getByLabelText("Motion")).toHaveValue("half");
    expect(document.querySelector(".workbench-shell")).toHaveAttribute(
      "data-motion-preset",
      "half",
    );
  });

  it("moves between storyboard steps with arrow keys", async () => {
    render(<App />);

    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});
    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "OpenGL state machine",
      );
    });

    fireEvent.keyDown(document.body, {key: "ArrowDown", bubbles: true});
    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "Vulkan PSO",
      );
    });

    fireEvent.keyDown(document.body, {key: "ArrowLeft", bubbles: true});
    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "OpenGL state machine",
      );
    });

    fireEvent.keyDown(document.body, {key: "ArrowUp", bubbles: true});
    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "A -> f(x) -> B",
      );
    });
  });

  it("renders the current storyboard step as the expanded rail item", () => {
    render(<App />);

    expect(document.querySelectorAll('.progress-bubble[data-compact="true"]').length).toBeGreaterThanOrEqual(2);
    expect(
      document.querySelector('.progress-bubble[data-state="current"][data-compact="false"]'),
    ).toBeInTheDocument();
    expect(
      document.querySelector('.progress-step-shell[data-state="current"] .progress-bubble[data-single-line="true"]'),
    ).toBeInTheDocument();
    expect(
      document.querySelector('.progress-step-shell[data-state="current"][data-layout="inline"]'),
    ).toBeInTheDocument();
    expect(
      document.querySelector('.progress-step-shell[data-state="current"][data-size-mode="expanded"]'),
    ).toBeInTheDocument();
    expect(
      document.querySelectorAll('.progress-step-shell[data-size-mode="compact"]').length,
    ).toBeGreaterThanOrEqual(2);
  });

  it("does not programmatically smooth-scroll the step rail during keyboard navigation", async () => {
    const originalScrollIntoView = HTMLElement.prototype.scrollIntoView;
    const scrollIntoView = vi.fn();
    HTMLElement.prototype.scrollIntoView = scrollIntoView;

    try {
      render(<App />);

      fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});

      await waitFor(() => {
        expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
          "OpenGL state machine",
        );
      });

      expect(scrollIntoView).not.toHaveBeenCalled();
    } finally {
      HTMLElement.prototype.scrollIntoView = originalScrollIntoView;
    }
  });

  it("renders manuscript content and code legends for lecture-driven steps", async () => {
    render(<App />);

    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});

    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "OpenGL state machine",
      );
    });

    expect(
      screen.getByText(
        "OpenGL lacks a single PSO, so applications layer shader and plumbing state piece by piece.",
      ),
    ).toBeInTheDocument();
    expect(
      screen.getByText(
        "Use two colors in the code block: one for shader stage setups and one for fixed pipeline states. Each call is independent.",
      ),
    ).toBeInTheDocument();
    expect(
      screen.getByText("glUseProgram(shaderProgram); // shader stage"),
    ).toBeInTheDocument();
    expect(screen.getByText("Shader stage binds")).toBeInTheDocument();
    expect(screen.getByText("Fixed pipeline tweaks")).toBeInTheDocument();
  });

  it("animates the manuscript card as a pull-away stack transition", async () => {
    render(<App />);

    expect(document.querySelectorAll(".notes-card-layer")).toHaveLength(2);
    expect(
      document.querySelector(
        '.notes-card-layer--outgoing[data-has-step="false"][data-stack-role="back"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.notes-card-layer--current[data-step-id="base_formula"][data-stack-role="front"]',
      ),
    ).toBeInTheDocument();

    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});

    await waitFor(() => {
      expect(screen.getByLabelText("Speaker notes")).toHaveAttribute(
        "data-motion-direction",
        "forward",
      );
    });
    expect(
      document.querySelector(
        '.notes-card-layer--outgoing[data-step-id="base_formula"][data-motion-direction="forward"][data-stack-role="front"][data-has-step="true"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector('.notes-card-layer--outgoing[data-fade="off"]'),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.notes-card-layer--outgoing[data-motion-axis="vertical"][data-step-id="base_formula"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.notes-card-layer--current[data-step-id="opengl_state_machine"][data-stack-role="back"][data-motion-direction="idle"][data-motion-axis="vertical"]',
      ),
    ).toBeInTheDocument();

    await waitFor(() => {
      expect(screen.getByLabelText("Speaker notes")).toHaveAttribute(
        "data-motion-direction",
        "idle",
      );
    });

    fireEvent.keyDown(document.body, {key: "ArrowLeft", bubbles: true});

    await waitFor(() => {
      expect(screen.getByLabelText("Speaker notes")).toHaveAttribute(
        "data-motion-direction",
        "backward",
      );
    });
    expect(
      document.querySelector(
        '.notes-card-layer--outgoing[data-step-id="opengl_state_machine"][data-motion-direction="backward"][data-stack-role="front"][data-has-step="true"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.notes-card-layer--current[data-step-id="base_formula"][data-stack-role="back"][data-motion-direction="idle"][data-motion-axis="vertical"]',
      ),
    ).toBeInTheDocument();
  });

  it("keeps pulling the current front card away during rapid step changes", async () => {
    render(<App />);

    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});
    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});

    await waitFor(() => {
      expect(screen.getByRole("heading", {level: 1})).toHaveTextContent(
        "Vulkan PSO",
      );
    });

    expect(
      document.querySelector(
        '.notes-card-layer--outgoing[data-step-id="base_formula"][data-motion-direction="forward"][data-stack-role="front"][data-has-step="true"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.notes-card-layer--current[data-step-id="vulkan_pso"][data-stack-role="back"][data-motion-direction="idle"]',
      ),
    ).toBeInTheDocument();
  });

  it("restores the rail collapse animation while keeping the row centered", async () => {
    render(<App />);

    fireEvent.keyDown(document.body, {key: "ArrowRight", bubbles: true});

    await waitFor(() => {
      expect(
        document.querySelector(
          '.progress-step-shell[data-step-id="opengl_state_machine"][data-size-mode="expanding"]',
        ),
      ).toBeInTheDocument();
    });

    expect(
      document.querySelector(
        '.progress-step-shell[data-step-id="base_formula"][data-size-mode="collapsing"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.progress-step-shell[data-step-id="base_formula"][data-size-mode="collapsing"][data-layout="compact"] .progress-bubble[data-size-mode="collapsing"][data-compact="true"][data-shape-mode="dot"]',
      ),
    ).toBeInTheDocument();
    expect(
      document.querySelector(
        '.progress-bubble[data-size-mode="compact"][data-compact="true"][data-shape-mode="dot"]',
      ),
    ).toBeInTheDocument();
  });

  it("reserves a fixed rail lane so footer animation does not shift the stage", () => {
    render(<App />);

    expect(document.querySelector(".workbench-shell")).toHaveStyle(
      "--rail-frame-height: 104px",
    );
    expect(document.querySelector(".workbench-shell")).toHaveStyle(
      "--rail-speed-factor: 0.75",
    );
    expect(document.querySelector(".progress-rail-window")).toBeInTheDocument();
  });

  it("keeps the progress row visually centered while step widths change", () => {
    render(<App />);

    expect(
      document.querySelector('.progress-bubbles[data-cross-align="centered"]'),
    ).toBeInTheDocument();
  });

  it("supports keyboard motion debugging shortcuts", async () => {
    render(<App />);

    fireEvent.keyDown(document.body, {key: "[", bubbles: true});
    await waitFor(() => {
      expect(document.querySelector(".workbench-shell")).toHaveAttribute(
        "data-motion-preset",
        "half",
      );
    });

    fireEvent.keyDown(document.body, {key: "[", bubbles: true});
    await waitFor(() => {
      expect(document.querySelector(".workbench-shell")).toHaveAttribute(
        "data-motion-preset",
        "quarter",
      );
    });

    fireEvent.keyDown(document.body, {key: "]", bubbles: true});
    await waitFor(() => {
      expect(document.querySelector(".workbench-shell")).toHaveAttribute(
        "data-motion-preset",
        "half",
      );
    });

    fireEvent.keyDown(document.body, {key: "0", bubbles: true});
    await waitFor(() => {
      expect(document.querySelector(".workbench-shell")).toHaveAttribute(
        "data-motion-preset",
        "normal",
      );
    });
  });

  it("does not hijack arrow keys while a select is focused", async () => {
    render(<App />);
    fireEvent.click(screen.getByRole("button", {name: "Show controls"}));

    const variantSelect = screen.getByLabelText("Variant");
    variantSelect.focus();

    fireEvent.keyDown(variantSelect, {key: "ArrowRight", bubbles: true});

    expect(screen.getByLabelText("Step")).toHaveValue("base_formula");
  });
});
