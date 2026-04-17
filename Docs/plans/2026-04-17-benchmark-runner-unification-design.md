# Benchmark Runner Unification Design

## Summary

`BenchmarkApp` already has working benchmark binaries and partial platform launchers, but the user-facing workflow is inconsistent. Some platforms emit raw JSON and CSV into the current directory, some require manual install steps, and there is no standard result bundle that records which device ran the benchmark and when it ran.

This work adds a thin runner layer around the existing benchmark outputs. Each supported platform should have a single script that can launch the benchmark, collect the JSON and CSV outputs, and store them in a timestamped result directory with a short metadata summary.

## Goals

- Give each platform a single entry script for real benchmark execution.
- Standardize result collection into one layout per run.
- Record device name and run time for every collected result bundle.
- Keep the existing benchmark JSON and CSV formats unchanged.
- Support iOS real-device installation and result pull via host-side automation.
- Provide an optional `.ipa` packaging script for iOS distribution workflows.

## Non-Goals

- No redesign of the benchmark core result schema.
- No GUI automation on iOS or Android beyond install and launch.
- No cross-host Windows verification from the current macOS machine.
- No remote lab orchestration or multi-device scheduling in this pass.

## Proposed Result Layout

Each runner stores outputs under:

```text
benchmark_results/<platform>/<timestamp>/
```

Each run directory contains:

- `benchmark_report.json`
- `compression_results.csv`
- `run_info.txt`

`run_info.txt` is the shared human-readable summary and must include:

- platform
- device name
- device identifier when available
- start time in UTC
- finish time in UTC
- source app or binary path

## Runner Strategy

### Shared Helper Layer

A small shell helper should handle:

- UTC timestamp creation
- result directory creation
- summary file writing

This keeps macOS, Android, and iOS scripts aligned without changing the benchmark binaries themselves.

### macOS

Add a dedicated macOS runner script that:

- runs `benchmark_main`
- writes JSON and CSV directly into the standardized result directory
- records host machine metadata in `run_info.txt`

### Android

Upgrade the existing Android runner script so it:

- detects the connected device name and serial
- pushes the built binary
- runs it on the device
- pulls JSON and CSV into the standardized result directory
- writes `run_info.txt`

### iOS

Add a host-side iOS runner script that:

- installs a signed `.app` to a selected physical device using `xcrun devicectl`
- launches the benchmark app
- polls the app data container until result files appear
- copies the result files back to the host
- writes `run_info.txt`

The script should prefer structured `devicectl --json-output` parsing and accept an explicit device identifier override so the user can target a specific phone or tablet.

### Windows

Add a PowerShell runner script mirroring the macOS result layout:

- run the Windows CLI benchmark executable
- place results in the standardized result directory
- write `run_info.txt`

This can be authored now even if it cannot be executed from the current macOS host.

### iOS `.ipa`

The build output remains a `.app` because that is what `xcodebuild` and `devicectl install app` use directly. A small packaging script should optionally wrap that `.app` into a `.ipa` for sharing or manual install workflows.

## Testing Strategy

The reusable shell logic should be covered by shell tests before implementation:

- result bundle helper test
- iOS device selection helper test

Platform scripts can then be verified with fresh smoke runs:

- macOS runner end-to-end
- Android build plus runner script static sanity
- iOS device package build plus runner command wiring sanity

