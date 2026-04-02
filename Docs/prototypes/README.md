# PSO Prototype Workbench

This directory now serves two roles:

- the original animation stack bakeoff
- the current browser-first workbench for fast iteration

## Active workflow

The current main entry point is `workbench`, not offline rerendering.

What the workbench does:

- keeps one slide-friendly host shell
- switches between animation libraries
- switches between per-library layout variants
- keeps the step bubbles, notes panel, and current story beat stable

Current library handling:

- `Remotion`: live inside the workbench host
- `Motion Canvas`: live via an embed surface on port `4175`
- `Manim`: reference mode with canonical step mapping

## Start the workbench

```bash
cd Docs/prototypes
npm install
npm run dev
```

Ports:

- workbench host: `http://127.0.0.1:4173`
- Motion Canvas embed runtime: `http://127.0.0.1:4175/embed.html`

## Build the current web stack

```bash
cd Docs/prototypes
npm run build
```

## Project layout

- `workbench`
  - unified host shell
  - notes panel
  - bottom progress bubbles
  - adapter layer
- `shared`
  - canonical storyboard contract
  - shared library and step types
- `remotion-demo`
  - live runtime rendered directly inside the host
- `motion-canvas-demo`
  - embed runtime for iframe-based comparison
- `manim-demo`
  - reference media and theory-heavy comparison path

## Canonical storyboard

All libraries map onto the same four steps:

1. `A -> f(x) -> B`
2. `Open f(x)`
3. `Inline material`
4. `Shared code`

## Visual constraints

- near-white slide-friendly background
- stable skeleton before expansion
- rounded boxes and orthogonal lines
- restrained technical tone
- no bounce or elastic motion
- emphasis by structure first, color second
