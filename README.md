# MT PSO Talk

This repository now has two product-facing roots and one documentation root:

- `SlideApp/`: the interactive slide experience for the PSO talk. It is a static Vite + React + Remotion Player app and is intended to publish to GitHub Pages.
- `BenchmarkApp/`: the benchmark harness for compression, Mesa/native graphics API timing, and platform runners.
- `Docs/`: talk notes, PPT material, plans, and durable reference assets.

## Quick Start

```bash
npm --prefix SlideApp install
npm run slide:dev
```

The app will start on `http://127.0.0.1:4173/`.

## SlideApp Commands

```bash
npm run slide:test
npm run slide:build
```

For GitHub Pages builds, the workflow sets `BASE_PATH` automatically so the static output works under `/<repo>/`.

## BenchmarkApp Configure Smoke Test

```bash
npm run benchmark:configure
```
