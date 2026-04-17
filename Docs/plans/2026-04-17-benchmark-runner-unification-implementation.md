# Benchmark Runner Unification Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add one-command benchmark runner scripts per platform and standardize every collected run into a timestamped result bundle with device and time metadata.

**Architecture:** Keep the benchmark binaries and app outputs unchanged, then add a shared shell helper for result bundle creation plus platform-specific wrappers for macOS, Android, iOS, and Windows. Use `xcrun devicectl` for iOS real-device install, launch, and result copy, and keep the iOS `.ipa` export as an optional packaging step.

**Tech Stack:** Bash, PowerShell, CMake/CTest shell tests, `adb`, `xcrun devicectl`, `xcodebuild`.

---

### Task 1: Add failing shell tests for the shared runner helper

**Files:**
- Create: `BenchmarkApp/tests/platform/result_bundle_test.sh`
- Modify: `BenchmarkApp/tests/CMakeLists.txt`

**Step 1: Write the failing test**

Cover:

- result directories land under `benchmark_results/<platform>/<timestamp>`
- the helper can write `run_info.txt`
- the summary includes platform, device, start time, and finish time

**Step 2: Run the focused test to verify it fails**

Run: `bash BenchmarkApp/tests/platform/result_bundle_test.sh`
Expected: FAIL because the shared helper does not exist yet.

### Task 2: Add a failing shell test for iOS device selection

**Files:**
- Create: `BenchmarkApp/tests/platform/ios_device_utils_test.sh`

**Step 1: Write the failing test**

Cover:

- selecting the first available device from `devicectl` JSON
- returning failure when only unavailable devices are present
- reading the bundle identifier from an app `Info.plist`

**Step 2: Run the focused test to verify it fails**

Run: `bash BenchmarkApp/tests/platform/ios_device_utils_test.sh`
Expected: FAIL because the iOS helper script does not exist yet.

### Task 3: Implement the shared result bundle helper

**Files:**
- Create: `BenchmarkApp/platform/common/result_bundle.sh`

**Step 1: Write the minimal helper**

Implement:

- UTC timestamp helper
- result directory creation helper
- `run_info.txt` writer

**Step 2: Re-run focused tests**

Run:
- `bash BenchmarkApp/tests/platform/result_bundle_test.sh`

Expected: PASS

### Task 4: Implement iOS device helper utilities

**Files:**
- Create: `BenchmarkApp/platform/ios/ios_device_utils.sh`

**Step 1: Write the minimal helper**

Implement:

- select available device from `devicectl` JSON
- read device name and UDID from `devicectl` JSON
- extract `CFBundleIdentifier` from an app bundle `Info.plist`

**Step 2: Re-run focused tests**

Run:
- `bash BenchmarkApp/tests/platform/ios_device_utils_test.sh`

Expected: PASS

### Task 5: Add platform runner scripts

**Files:**
- Create: `BenchmarkApp/platform/macos/run_benchmark.sh`
- Modify: `BenchmarkApp/platform/android/run_benchmark.sh`
- Create: `BenchmarkApp/platform/ios/run_benchmark.sh`
- Create: `BenchmarkApp/platform/ios/package_ios_ipa.sh`
- Create: `BenchmarkApp/platform/windows/run_benchmark.ps1`

**Step 1: Write the minimal platform wrappers**

Each runner should:

- create a standardized result directory
- run or launch the benchmark
- collect JSON and CSV into that directory
- write `run_info.txt`
- print a short completion summary with device and time

**Step 2: Keep names aligned**

Normalize collected file names to:

- `benchmark_report.json`
- `compression_results.csv`

### Task 6: Update docs

**Files:**
- Modify: `BenchmarkApp/README.md`
- Modify: `README.md`

**Step 1: Document the one-command platform runners**

Describe:

- macOS runner
- Android build + runner
- iOS build + runner
- optional iOS `.ipa` packaging
- Windows PowerShell runner
- standardized result directory layout

### Task 7: Run verification

**Files:**
- Verify only

**Step 1: Run focused shell tests**

Run:
- `bash BenchmarkApp/tests/platform/result_bundle_test.sh`
- `bash BenchmarkApp/tests/platform/ios_device_utils_test.sh`

Expected: PASS

**Step 2: Run the CTest suite**

Run:
- `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-runners`
- `cmake --build /tmp/mt-pso-benchmark-runners -j4`
- `ctest --test-dir /tmp/mt-pso-benchmark-runners --output-on-failure`

Expected: PASS

**Step 3: Run the macOS runner**

Run:
- `BenchmarkApp/platform/macos/run_benchmark.sh /tmp/mt-pso-benchmark-runners/lib/benchmark_main`

Expected: PASS and create a timestamped result directory

**Step 4: Reconfirm mobile packaging**

Run:
- `./BenchmarkApp/platform/android/build_android.sh`
- `IOS_DEVELOPMENT_TEAM=5PGYZ56HGA IOS_BUNDLE_IDENTIFIER=com.swannzhang.psobenchmark ./BenchmarkApp/platform/ios/build_ios.sh device`

Expected: PASS
