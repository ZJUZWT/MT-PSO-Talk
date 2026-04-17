# Cross-Shell Benchmark Runners Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add Android and iOS PowerShell benchmark runners and package them into the repository-level `release/` directory.

**Architecture:** Keep the existing shell runners as-is and add parallel `ps1` launchers for Android and iOS. The Android PowerShell runner will mirror the current adb push/run/pull workflow. The iOS PowerShell runner will mirror the current `xcrun devicectl` workflow and explicitly guard against non-macOS execution.

**Tech Stack:** Bash, PowerShell, adb, xcrun devicectl, CMake shell tests.

---

### Task 1: Lock the release contract with a failing test

**Files:**
- Modify: `BenchmarkApp/tests/platform/release_assembly_test.sh`
- Test: `BenchmarkApp/tests/platform/release_assembly_test.sh`

**Step 1: Write the failing test**

Add expectations for:

```bash
"$RELEASE_ROOT/scripts/run_android.ps1"
"$RELEASE_ROOT/scripts/run_ios.ps1"
```

**Step 2: Run test to verify it fails**

Run: `bash BenchmarkApp/tests/platform/release_assembly_test.sh`
Expected: FAIL because the release directory does not yet contain the new PowerShell scripts.

**Step 3: Write minimal implementation**

Create the Android and iOS `ps1` runners and copy them into the assembled release directory.

**Step 4: Run test to verify it passes**

Run: `bash BenchmarkApp/tests/platform/release_assembly_test.sh`
Expected: PASS.

### Task 2: Implement Android and iOS PowerShell runners

**Files:**
- Create: `BenchmarkApp/platform/android/run_benchmark.ps1`
- Create: `BenchmarkApp/platform/ios/run_benchmark.ps1`
- Modify: `BenchmarkApp/platform/release/assemble_release.sh`

**Step 1: Write the failing test**

Reuse the failing release assembly test from Task 1.

**Step 2: Run test to verify it fails**

Run: `bash BenchmarkApp/tests/platform/release_assembly_test.sh`
Expected: FAIL until both scripts are added and copied.

**Step 3: Write minimal implementation**

- Android `ps1`: detect `adb`, push the packaged binary, run it, pull JSON and CSV, and write `run_info.txt`.
- iOS `ps1`: require macOS + `xcrun`, install and launch the `.app`, poll the app data container for JSON and CSV, and write `run_info.txt`.
- Release assembler: include the new `ps1` launchers in `release/scripts/`.

**Step 4: Run test to verify it passes**

Run: `bash BenchmarkApp/tests/platform/release_assembly_test.sh`
Expected: PASS.

### Task 3: Document and verify the full matrix

**Files:**
- Modify: `BenchmarkApp/README.md`
- Modify: `README.md`

**Step 1: Write the failing test**

Use the release assembly test plus a fresh configured test suite run as the verification gate.

**Step 2: Run test to verify current state**

Run: `ctest --test-dir /tmp/mt-pso-benchmark-release --output-on-failure`
Expected: Existing tests pass once the temporary build tree is configured and built.

**Step 3: Write minimal implementation**

Document the new `ps1` entry points and clarify that iOS PowerShell execution still requires macOS.

**Step 4: Run test to verify it passes**

Run: `rm -rf /tmp/mt-pso-cross-shell && cmake -S BenchmarkApp -B /tmp/mt-pso-cross-shell && cmake --build /tmp/mt-pso-cross-shell -j4 && ctest --test-dir /tmp/mt-pso-cross-shell --output-on-failure`
Expected: PASS with all tests green.
