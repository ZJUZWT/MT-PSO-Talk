import {render, screen} from "@testing-library/react";
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
});
