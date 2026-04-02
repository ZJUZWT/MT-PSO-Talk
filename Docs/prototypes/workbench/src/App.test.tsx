import {render, screen} from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import {describe, expect, it} from "vitest";
import {App} from "./App";

describe("App", () => {
  it("shows the current step and all progress bubbles", () => {
    render(<App />);

    expect(
      screen.getByRole("heading", {name: "A -> f(x) -> B", level: 1}),
    ).toBeInTheDocument();
    expect(screen.getAllByRole("listitem")).toHaveLength(4);
  });

  it("falls back to the supported Manim state when switching libraries", async () => {
    const user = userEvent.setup();

    render(<App />);

    await user.selectOptions(screen.getByLabelText("Step"), "open_fx");
    await user.selectOptions(screen.getByLabelText("Library"), "manim");

    expect(screen.getByLabelText("Variant")).toHaveValue("reference");
    expect(screen.getByLabelText("Step")).toHaveValue("base_formula");
  });
});
