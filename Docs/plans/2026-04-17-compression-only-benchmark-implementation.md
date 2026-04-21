# Compression-Only Benchmark Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Turn `BenchmarkApp` into a compression-only benchmark that measures compression and decompression for all supported algorithms across macOS, Windows, Android, and iOS, with Oodle backed by Unreal Engine `OodleDataCompression` SDK assets.

**Architecture:** Keep the shared C++ compression engine and result exporters, but make the orchestrator and platform runners compression-first. Vendor Unreal Engine `OodleDataCompression/Sdks/2.9.12` into `BenchmarkApp/third_party/oodle/2.9.12`, teach CMake to link the correct `oo2core` library per platform, and update tests so default product behavior no longer depends on graphics or Mesa paths.

**Tech Stack:** C++17, CMake, CTest, Android NDK, Objective-C++ for iOS shell, vendored zstd/lz4/zlib/snappy/brotli, Unreal Engine OodleDataCompression SDK `2.9.12`.

---

### Task 1: Document the compression-only product direction

**Files:**
- Create: `Docs/plans/2026-04-17-compression-only-benchmark-design.md`
- Create: `Docs/plans/2026-04-17-compression-only-benchmark-implementation.md`

**Step 1: Write the design and implementation plan docs**

Use the approved compression-only scope, Oodle vendoring strategy, and four-platform delivery goals.

**Step 2: Sanity-check paths**

Run: `test -f Docs/plans/2026-04-17-compression-only-benchmark-design.md && test -f Docs/plans/2026-04-17-compression-only-benchmark-implementation.md`
Expected: success

### Task 2: Write failing orchestrator tests for compression-only defaults

**Files:**
- Modify: `BenchmarkApp/tests/orchestrator/orchestrator_test.cpp`
- Test: `BenchmarkApp/tests/orchestrator/orchestrator_test.cpp`

**Step 1: Write the failing test**

Add expectations that:

- default orchestrator run produces no graphics product output
- compression results remain non-empty
- summary text focuses on compression-only wording
- no graphics backend names are required in the default report

**Step 2: Run the focused test to verify it fails**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target orchestrator_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R orchestrator_test --output-on-failure`
Expected: FAIL because the orchestrator still emits graphics-first output.

### Task 3: Write failing tests for algorithm availability reporting

**Files:**
- Create: `BenchmarkApp/tests/compression/algorithm_availability_test.cpp`
- Modify: `BenchmarkApp/tests/CMakeLists.txt`
- Test: `BenchmarkApp/tests/compression/algorithm_availability_test.cpp`

**Step 1: Write the failing test**

Cover:

- registry exposes built-in algorithms
- Oodle availability can be queried without forcing execution
- summary metadata distinguishes unavailable from failed

**Step 2: Run the focused test to verify it fails**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target algorithm_availability_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R algorithm_availability_test --output-on-failure`
Expected: FAIL because the availability surface does not exist yet.

### Task 4: Implement compression-only orchestrator behavior

**Files:**
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_orchestrator.h`
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_orchestrator.cpp`
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_main.cpp`

**Step 1: Write minimal implementation**

Change the default orchestrator run path so it:

- skips graphics execution unless explicitly requested
- runs compression with platform-aware labels instead of `"Simulated"`
- formats compression-first summary text
- writes only the compression report sections in product-facing CLI output

**Step 2: Run focused tests**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target orchestrator_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R orchestrator_test --output-on-failure`
Expected: PASS

### Task 5: Implement algorithm availability and optional Oodle enablement

**Files:**
- Modify: `BenchmarkApp/lib/compression/compression_benchmark.h`
- Modify: `BenchmarkApp/lib/compression/compression_benchmark.cpp`
- Modify: `BenchmarkApp/lib/compression/algorithms/compression_algorithm.h`
- Modify: `BenchmarkApp/lib/results/schema/result_types.h`
- Modify: `BenchmarkApp/lib/results/schema/result_json.cpp`
- Modify: `BenchmarkApp/lib/results/exporters/csv_exporter.cpp`

**Step 1: Write minimal implementation**

Add the smallest API needed to:

- enumerate available algorithms for the current build
- mark skipped algorithms cleanly in summary/report layers
- preserve current round-trip timing logic for runnable algorithms

**Step 2: Run focused tests**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target algorithm_availability_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R algorithm_availability_test --output-on-failure`
Expected: PASS

### Task 6: Vendor Unreal Engine OodleDataCompression SDK 2.9.12

**Files:**
- Modify: `BenchmarkApp/third_party/CMakeLists.txt`
- Modify: `BenchmarkApp/lib/CMakeLists.txt`
- Replace: `BenchmarkApp/third_party/oodle/include/*`
- Replace: `BenchmarkApp/third_party/oodle/lib/*`

**Step 1: Copy the approved SDK payload**

Use Unreal Engine source assets from:

- `/Users/swannzhang/Workspace/CppProjects/UnrealEngine/Engine/Source/Runtime/OodleDataCompression/Sdks/2.9.12/include`
- `/Users/swannzhang/Workspace/CppProjects/UnrealEngine/Engine/Source/Runtime/OodleDataCompression/Sdks/2.9.12/lib`

**Step 2: Update CMake**

Teach `tp_oodle` to select:

- `Mac/liboo2coremac64.a`
- `Win64/oo2core_win64.lib`
- `IOS/liboo2coreios.a`
- `Android/<abi>/liboo2coreandroid.a`

Keep the target optional on unsupported platforms instead of failing globally.

**Step 3: Run configure/build to verify Oodle wiring**

Run: `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
Expected: PASS

### Task 7: Update Oodle algorithm implementation to match the vendored SDK

**Files:**
- Modify: `BenchmarkApp/lib/compression/algorithms/oodle_algorithm.cpp`
- Modify: `BenchmarkApp/tests/compression/compression_algorithm_test.cpp`

**Step 1: Write the failing test**

Add a focused Oodle smoke test guarded by availability so it proves the algorithm can be constructed and round-tripped when the SDK is present.

**Step 2: Run the focused test to verify it fails**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target compression_algorithm_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R compression_algorithm_test --output-on-failure`
Expected: FAIL or skip mismatch until Oodle include/library wiring is corrected.

**Step 3: Write minimal implementation**

Keep the current `OodleLZ_Compress` / `OodleLZ_Decompress` implementation and adjust only what is required for the new include/library layout and availability checks.

**Step 4: Re-run the focused test**

Run: `cmake --build /tmp/mt-pso-benchmark-check --target compression_algorithm_test && ctest --test-dir /tmp/mt-pso-benchmark-check -R compression_algorithm_test --output-on-failure`
Expected: PASS

### Task 8: Update platform runners for compression-only product behavior

**Files:**
- Modify: `BenchmarkApp/platform/android/benchmark_cli.cpp`
- Modify: `BenchmarkApp/platform/android/run_benchmark.sh`
- Modify: `BenchmarkApp/platform/ios/benchmark_app.mm`
- Add: `BenchmarkApp/platform/windows/CMakeLists.txt`
- Add: `BenchmarkApp/platform/windows/benchmark_cli.cpp`
- Modify: `BenchmarkApp/platform/CMakeLists.txt`

**Step 1: Write the failing test or smoke expectation**

Define smoke expectations that platform entrypoints:

- no longer print graphics matrices by default
- still emit JSON/CSV compression output
- identify the active platform correctly

**Step 2: Implement the minimal platform changes**

Keep the runners thin. They should call the shared compression-only orchestrator and handle output paths.

**Step 3: Run platform configure smoke checks**

Run:
- `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
- `cmake --build /tmp/mt-pso-benchmark-check --target benchmark_main`
- `cmake --build /tmp/mt-pso-benchmark-check --target pso_benchmark` when supported

Expected: PASS for available targets on the current host.

### Task 9: Update docs and README copy to reflect the new product

**Files:**
- Modify: `BenchmarkApp/README.md`
- Modify: `README.md`

**Step 1: Write the failing doc expectation**

Search output should stop describing `BenchmarkApp` primarily as a graphics benchmark harness.

**Step 2: Update the docs**

Describe:

- compression-only benchmark positioning
- supported platforms
- Oodle vendoring expectations
- main configure/build commands

**Step 3: Verify the wording**

Run: `rg -n "Mesa|Metal PSO|graphics API timing|compression-only|OodleDataCompression" README.md BenchmarkApp/README.md`
Expected: new compression-first wording present, stale product-facing graphics language removed from the main docs.

### Task 10: Run full verification before completion

**Files:**
- Verify only

**Step 1: Reconfigure**

Run: `cmake -S BenchmarkApp -B /tmp/mt-pso-benchmark-check`
Expected: PASS

**Step 2: Rebuild the benchmark test suite**

Run: `cmake --build /tmp/mt-pso-benchmark-check -j4`
Expected: PASS

**Step 3: Run focused benchmark tests**

Run: `ctest --test-dir /tmp/mt-pso-benchmark-check --output-on-failure`
Expected: all relevant benchmark tests PASS on the current host

**Step 4: Run the desktop compression binary**

Run: `/tmp/mt-pso-benchmark-check/lib/benchmark_main --json /tmp/benchmark_report.json --csv`
Expected: exit 0 and compression report output
