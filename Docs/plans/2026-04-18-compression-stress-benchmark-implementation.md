# Compression Stress Benchmark Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Broaden the compression benchmark into a multi-profile, multi-size, stronger-validation stress run that is credible for cross-device comparison.

**Architecture:** Extend compression configuration and result metadata so the benchmark can run deterministic payload families across several sizes and iterations. Keep the current orchestrator and release flows, but surface the richer benchmark shape in CLI output, JSON, CSV, and summaries so testers can actually see which workload groups were measured.

**Tech Stack:** C++, Bash/PowerShell runner scripts, CMake/CTest.

---

### Task 1: Lock richer compression metadata with failing tests

**Files:**
- Modify: `BenchmarkApp/tests/compression/compression_benchmark_test.cpp`
- Modify: `BenchmarkApp/tests/results/result_json_test.cpp`
- Modify: `BenchmarkApp/tests/results/exporter_test.cpp`
- Modify: `BenchmarkApp/tests/orchestrator/orchestrator_test.cpp`

**Step 1: Write the failing test**

Add expectations for:

```cpp
result.payload_profile == "custom"
result.iteration_index == 0
!result.input_hash.empty()
result.roundtrip_hash_match
result.roundtrip_byte_match
```

Also add a deliberately corrupt algorithm in `compression_benchmark_test.cpp` that decompresses to the wrong bytes and must return:

```cpp
status == "failed"
roundtrip_hash_match == false
roundtrip_byte_match == false
```

**Step 2: Run test to verify it fails**

Run: `cmake --build /tmp/mt-pso-stress -j4 --target compression_benchmark_test result_json_test exporter_test orchestrator_test && ctest --test-dir /tmp/mt-pso-stress --output-on-failure -R "compression_benchmark_test|result_json_test|exporter_test|orchestrator_test"`
Expected: FAIL to compile or fail assertions because the new fields and behavior do not exist yet.

**Step 3: Write minimal implementation**

Add the new compression-result metadata and populate it.

**Step 4: Run test to verify it passes**

Run the same command.
Expected: PASS.

### Task 2: Expand compression benchmark coverage

**Files:**
- Modify: `BenchmarkApp/lib/compression/compression_benchmark.h`
- Modify: `BenchmarkApp/lib/compression/compression_benchmark.cpp`
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_orchestrator.h`
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_orchestrator.cpp`

**Step 1: Write the failing test**

Use the updated benchmark and orchestrator tests to require:
- multiple payload profiles
- multiple payload sizes
- iteration metadata
- matrix output that includes payload profile names

**Step 2: Run test to verify it fails**

Run: `ctest --test-dir /tmp/mt-pso-stress --output-on-failure -R "compression_benchmark_test|orchestrator_test"`
Expected: FAIL.

**Step 3: Write minimal implementation**

- Add payload-profile config to compression benchmark and orchestrator config.
- Generate deterministic payloads for `pso_like`, `high_compressibility`, and `low_compressibility`.
- Add optional warmup iterations.
- Update the compression matrix formatter to emit sections for every `(payload_profile, input_size)` group instead of only the first payload size.

**Step 4: Run test to verify it passes**

Run the same command.
Expected: PASS.

### Task 3: Surface the new benchmark controls in CLI and docs

**Files:**
- Modify: `BenchmarkApp/lib/orchestrator/benchmark_main.cpp`
- Modify: `BenchmarkApp/README.md`
- Modify: `README.md`
- Modify: `BenchmarkApp/platform/ios/run_benchmark.sh`
- Modify: `BenchmarkApp/platform/ios/run_benchmark.ps1`
- Modify: `BenchmarkApp/platform/release/assemble_release.sh`

**Step 1: Write the failing test**

Rely on the full benchmark suite plus release-assembly coverage to catch regressions after CLI and runner updates.

**Step 2: Run test to verify current state**

Run: `bash BenchmarkApp/tests/platform/release_assembly_test.sh`
Expected: PASS before the docs-only changes, then re-run after the script tweaks.

**Step 3: Write minimal implementation**

- Add CLI flags for compression iterations, warmup iterations, payload sizes, and payload profiles.
- Document the heavier default benchmark behavior.
- Increase iOS poll timeout headroom if the new default run becomes longer.

**Step 4: Run test to verify it passes**

Run: `rm -rf /tmp/mt-pso-stress && cmake -S BenchmarkApp -B /tmp/mt-pso-stress && cmake --build /tmp/mt-pso-stress -j4 && ctest --test-dir /tmp/mt-pso-stress --output-on-failure`
Expected: PASS.

