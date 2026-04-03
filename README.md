# MT PSO Talk

This repository now has two product-facing roots and one documentation root:

- `SlideApp/`: the interactive slide experience for the PSO talk. It now lives in the public repository `ZJUZWT/MT-PSO-Talk-SlideApp` and is mounted here as a git submodule at the same path.
- `BenchmarkApp/`: the benchmark harness for compression, Mesa/native graphics API timing, and platform runners.
- `Docs/`: talk notes, PPT material, plans, and durable reference assets.

## Quick Start

```bash
npm --prefix SlideApp ci
npm run slide:dev
```

The app will start on `http://127.0.0.1:4173/`.

If this is a fresh clone, initialize the submodule first:

```bash
git submodule update --init --recursive
```

## SlideApp Commands

```bash
npm run slide:test
npm run slide:build
```

Public repository:

`https://github.com/ZJUZWT/MT-PSO-Talk-SlideApp`

Public Pages site:

`https://zjuzwt.github.io/MT-PSO-Talk-SlideApp/`

For GitHub Pages builds in the public repo, the workflow sets `BASE_PATH` automatically so the static output works under `/<repo>/`.

## BenchmarkApp Configure Smoke Test

```bash
npm run benchmark:configure
```
