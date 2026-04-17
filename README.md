# MT PSO Talk

This repository now has two product-facing roots and one documentation root:

- `SlideApp/`: the interactive slide experience for the PSO talk. It now lives in the public repository `ZJUZWT/MT-PSO-Talk-SlideApp` and is mounted here as a git submodule at the same path.
- `BenchmarkApp/`: the compression benchmark harness for macOS, Windows, Android, and iOS. It measures real compression and decompression timings across all bundled algorithms, including Oodle where supported.
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

## BenchmarkApp Run Smoke Test

```bash
cmake -S BenchmarkApp -B build/benchmark
cmake --build build/benchmark -j4
BenchmarkApp/platform/macos/run_benchmark.sh ./build/benchmark/lib/benchmark_main
```

Result bundles are stored under `benchmark_results/<platform>/<timestamp>/` with:

- `benchmark_report.json`
- `compression_results.csv`
- `run_info.txt`

To gather the current benchmark packages and one-command runner scripts into a fixed repository-level `release/` directory, run:

```bash
BenchmarkApp/platform/release/assemble_release.sh
```

The assembled `release/scripts/` directory now includes both shell and PowerShell launchers for Android and iOS. Android can be driven from any machine with `pwsh` and `adb`; iOS still requires macOS because the runner depends on Xcode's `xcrun devicectl`.
