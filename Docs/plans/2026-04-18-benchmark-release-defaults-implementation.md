# Benchmark Release Defaults Implementation

## Problem

Cross-device compression comparisons briefly showed Android faster than Apple Silicon
macOS in almost every category. The benchmark logic itself was not the primary issue.
The results were skewed because the Android binary was built as `Release`, while the
macOS binary had been configured without `CMAKE_BUILD_TYPE`, which left a single-config
desktop build effectively unoptimized.

## Root Cause

- `BenchmarkApp/platform/android/build_android.sh` already passed `-DCMAKE_BUILD_TYPE=Release`.
- The macOS path relied on ad-hoc `cmake -S ... -B ...` commands with no equivalent default.
- Single-config generators therefore produced a much slower macOS benchmark binary.

## Changes

- Added a single-config CMake default so omitted `CMAKE_BUILD_TYPE` now resolves to `Release`.
- Added `BenchmarkApp/platform/macos/build_macos.sh` as the explicit macOS benchmark build entry point.
- Added `BenchmarkApp/platform/windows/build_windows.ps1` so Windows now has a first-class build entry point too.
- Added `BenchmarkApp/platform/common/build_settings.sh` so benchmark-oriented scripts share the same defaults for build root, build type, build jobs, and multi-config output paths.
- Updated iOS packaging and runner path resolution to honor the shared configuration defaults instead of hard-coding a single `Release` output path.
- Updated release assembly so Windows packages can be picked up from both single-config and multi-config build layouts.
- Updated macOS runner messaging so missing binaries point users at the new Release build script.
- Updated README guidance to require `Release` builds for cross-device benchmark comparisons.

## Shared Build Defaults

- `BUILD_ROOT`: `build/BenchmarkApp`
- `BENCHMARK_BUILD_TYPE`: `Release`
- `BENCHMARK_BUILD_JOBS`: `4`
- `BENCHMARK_IOS_CONFIGURATION`: optional override for iOS app bundle paths
- `BENCHMARK_WINDOWS_CONFIGURATION`: optional override for Windows multi-config output paths

## Verification

- Added regression coverage for single-config CMake defaulting to `Release`
- Added regression coverage for the macOS build script defaulting to `Release`
- Added regression coverage for shared build setting precedence
- Added regression coverage for iOS configuration-aware IPA packaging
- Added regression coverage for Windows multi-config release assembly

## Operational Rule

Use `Release` builds for every benchmark package that will be compared across devices.
Debug or otherwise unoptimized builds are allowed for local debugging, but they must not
be used to draw performance conclusions against Android, iOS, or other desktop runs.
