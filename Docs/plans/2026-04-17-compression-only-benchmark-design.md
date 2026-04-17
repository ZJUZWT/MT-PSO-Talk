# Compression-Only Benchmark Design

## Summary

This document redefines `BenchmarkApp` as a compression-focused benchmark product. The graphics, Mesa, and PSO timing paths remain in the repository only as legacy code until they are no longer referenced, but the product-facing runners, orchestrator defaults, and documentation will move to a compression-only workflow.

The new benchmark must run the same compression and decompression suite across macOS, Windows, Android, and iOS. All supported algorithms should be measured on every platform where their required libraries are available, with Oodle backed by Unreal Engine's `OodleDataCompression` SDK instead of the current single-platform vendor drop.

## Goals

- Make `BenchmarkApp` a compression-only benchmark by default.
- Measure both compression and decompression across all built-in algorithms.
- Support shared benchmark execution flows for:
  - macOS
  - Windows
  - Android
  - iOS
- Keep result formats comparable across platforms.
- Include Oodle in the algorithm matrix when platform libraries are available.
- Source Oodle headers and libraries from a vendored copy of Unreal Engine `OodleDataCompression` SDK version `2.9.12`.

## Non-Goals

- No Mesa or graphics API benchmark delivery in this work.
- No new PSO or shader timing instrumentation.
- No public-repo packaging work for proprietary Oodle assets.
- No Linux product runner work in this pass.

## Product Direction

`BenchmarkApp` becomes a CPU compression benchmark instead of a mixed graphics-plus-compression harness. The shared C++ benchmark core remains the center of the app, but the benchmark scheduler, reports, tests, and platform shells should all assume compression as the primary workload.

This reduces complexity in three ways:

- platform entrypoints no longer need graphics backends or GPU context setup
- reports no longer need to merge unrelated graphics and compression summaries
- Android and iOS support no longer depend on unfinished real-GPU paths

## Algorithm Matrix

### Always-On Algorithms

These algorithms are expected to run anywhere the project builds:

- `noop`
- `zstd`
- `lz4`
- `zlib`
- `snappy`
- `brotli`

### Conditional Algorithms

These algorithms are enabled when their SDK is available:

- `oodle_kraken`
- `oodle_leviathan`
- `oodle_mermaid`
- `oodle_selkie`

Oodle must not silently disappear. The build or runtime layer should expose whether Oodle support is compiled in, and reports should clearly show whether Oodle algorithms were executed or skipped.

## Oodle SDK Strategy

The benchmark already uses `oodle2.h` and `OodleLZ_Compress` / `OodleLZ_Decompress`, which map to the `oo2core` library family. The correct SDK source is Unreal Engine `Engine/Source/Runtime/OodleDataCompression/Sdks/2.9.12`, not the `OodleNetwork` plugin.

The project should vendor the following under `BenchmarkApp/third_party/oodle/2.9.12/`:

- `include/oodle2.h`
- `include/oodle2base.h`
- the `lib` subtree for:
  - `Android`
  - `IOS`
  - `Mac`
  - `Win64`

The benchmark CMake should select the matching `oo2core` library per target platform:

- Android: `liboo2coreandroid.a` for the active ABI
- iOS: `liboo2coreios.a` or simulator variant when needed
- macOS: `liboo2coremac64.a`
- Windows: `oo2core_win64.lib`

## Core Architecture

### Compression Engine

`CompressionBenchmark` remains the shared benchmark engine. It continues to:

- iterate over registered algorithms
- iterate over supported levels
- generate deterministic payloads
- record compression and decompression timings
- validate round-trip correctness

The engine should grow one capability: reporting which algorithms are currently available for the active build target, so platform summaries can distinguish "not enabled" from "failed while running".

### Orchestrator

`BenchmarkOrchestrator` should become compression-first:

- graphics execution disabled by default
- compression execution enabled by default
- summary text rewritten to focus on algorithms, ratios, and throughput
- output file names and CLI text aligned with compression-only runs

Backward-compatible graphics fields may remain in the result struct temporarily if removing them would make the patch too broad, but product-facing output should not mention graphics when no graphics run is requested.

### Platform Entry Points

All product-facing platform runners should share the same benchmark core and differ only in shell behavior:

- desktop CLI prints summary and optionally writes JSON/CSV
- Android CLI pushes and runs the native binary via `adb`
- iOS app triggers the compression suite and saves output to app documents
- Windows CLI uses the same C++ core as macOS

Each runner should surface:

- active platform name
- enabled algorithm count
- payload sizes
- iteration count
- JSON/CSV output path

## Result Model

The compression result schema is already close to the desired end state. The product-facing report should emphasize:

- algorithm
- level
- input size
- compressed size
- ratio
- compress time
- decompress time
- throughput
- status

The summary should answer:

- which algorithm compressed best
- which algorithm decompressed fastest
- which algorithms were skipped or unavailable

## Testing Strategy

All behavior changes should be TDD-driven.

### Unit and Integration Coverage

- orchestrator tests must stop expecting graphics by default
- orchestrator tests must verify compression-only summary text
- compression registry tests must verify Oodle availability is reported correctly
- CMake/config tests must verify Oodle selection paths per platform
- CLI tests or smoke checks must verify desktop compression target still configures and builds

### Verification Targets

- macOS configure/build succeeds for desktop compression runner
- existing compression tests still pass after refactor
- new compression-only orchestrator tests pass

## Migration Notes

- Legacy graphics code can stay in place as long as product-facing targets stop depending on it.
- The current `BenchmarkApp/third_party/oodle` tree should be replaced or reorganized so it no longer implies Mac-only support.
- README and root docs should describe `BenchmarkApp` as a compression benchmark tool first.
