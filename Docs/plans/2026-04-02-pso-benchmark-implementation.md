# Cross-Platform PSO and Compression Benchmark Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a test-only benchmark project under `PSO-compile-test/Test` that measures Android Mesa and native graphics API costs, iOS Metal API costs, and Windows compression baselines with mobile decompression replay.

**Architecture:** Use a shared C++17 benchmark core in `PSO-compile-test/Test` for manifests, case scheduling, timing, result schemas, and export. Add thin platform adapters for Android, iOS, and Windows, with Mesa-specific instrumentation adapters isolated under `PSO-compile-test/Test/Mesa` and platform-specific launch shells under `PSO-compile-test/Test/Platform`.

**Tech Stack:** C++17, CMake, CTest, Android NDK, Objective-C++ for Metal bridge code, JSON serialization, Mesa patch set plus trace adapter scripts.

---

### Task 1: Repair the project scaffold around a test-only benchmark layout

**Files:**
- Modify: `PSO-compile-test/CMakeLists.txt`
- Create: `PSO-compile-test/Test/CMakeLists.txt`
- Create: `PSO-compile-test/Test/TestSupport/assert.h`
- Create: `PSO-compile-test/Test/TestSupport/test_main.cpp`
- Create: `PSO-compile-test/Test/README.md`
- Test: `PSO-compile-test/Test/TestSupport/test_main.cpp`

**Step 1: Write the failing smoke test entrypoint**

```cpp
// PSO-compile-test/Test/TestSupport/test_main.cpp
#include "TestSupport/assert.h"

int main() {
    test_support::expect_true(true, "smoke");
    return test_support::finish();
}
```

**Step 2: Run configure to verify it fails**

Run: `cmake -S PSO-compile-test -B build/pso-benchmark`
Expected: FAIL because `PSO-compile-test/CMakeLists.txt` still references invalid placeholder paths and no `Test` subdirectory exists.

**Step 3: Write the minimal scaffold implementation**

```cmake
# PSO-compile-test/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(PSOBenchmark LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
add_subdirectory(Test)
```

```cmake
# PSO-compile-test/Test/CMakeLists.txt
add_library(test_support STATIC TestSupport/test_main.cpp)
target_include_directories(test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_support_smoke TestSupport/test_main.cpp)
target_include_directories(test_support_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_support_smoke COMMAND test_support_smoke)
```

```cpp
// PSO-compile-test/Test/TestSupport/assert.h
#pragma once

#include <iostream>
#include <string>

namespace test_support {
inline int& failure_count() {
    static int count = 0;
    return count;
}

inline void expect_true(bool value, const std::string& label) {
    if (!value) {
        std::cerr << "EXPECT_TRUE failed: " << label << "\n";
        ++failure_count();
    }
}

inline int finish() {
    return failure_count() == 0 ? 0 : 1;
}
}  // namespace test_support
```

**Step 4: Run the smoke test to verify it passes**

Run: `cmake -S PSO-compile-test -B build/pso-benchmark && cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark --output-on-failure`
Expected: PASS with `test_support_smoke` succeeding.

**Step 5: Commit**

```bash
git add PSO-compile-test/CMakeLists.txt PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/TestSupport/assert.h PSO-compile-test/Test/TestSupport/test_main.cpp PSO-compile-test/Test/README.md
git commit -m "build: scaffold test-only PSO benchmark layout"
```

### Task 2: Add the unified benchmark result schema and serialization tests

**Files:**
- Create: `PSO-compile-test/Test/Results/schema/result_types.h`
- Create: `PSO-compile-test/Test/Results/schema/result_types.cpp`
- Create: `PSO-compile-test/Test/Results/schema/result_json.h`
- Create: `PSO-compile-test/Test/Results/schema/result_json.cpp`
- Create: `PSO-compile-test/Test/Results/tests/result_json_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Results/tests/result_json_test.cpp`

**Step 1: Write the failing result serialization test**

```cpp
// PSO-compile-test/Test/Results/tests/result_json_test.cpp
#include "Results/schema/result_json.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::GraphicsResult result{};
    result.platform = "Android";
    result.api = "Vulkan";
    result.driver_mode = "Mesa";
    result.total_us = 42;
    const std::string json = benchmark::to_json(result);
    test_support::expect_true(json.find("\"platform\":\"Android\"") != std::string::npos, "platform serialized");
    test_support::expect_true(json.find("\"total_us\":42") != std::string::npos, "total serialized");
    return test_support::finish();
}
```

**Step 2: Run the focused test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R result_json_test --output-on-failure`
Expected: FAIL because the schema and serialization files do not exist.

**Step 3: Write the minimal schema and serializer**

```cpp
// PSO-compile-test/Test/Results/schema/result_types.h
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace benchmark {
struct StageTiming {
    std::string name;
    std::int64_t duration_us = 0;
};

struct GraphicsResult {
    std::string platform;
    std::string api;
    std::string driver_mode;
    std::string case_name;
    std::int64_t total_us = 0;
    std::vector<StageTiming> stage_breakdown;
};

struct CompressionResult {
    std::string platform;
    std::string algorithm;
    std::string baseline_package_id;
    std::int64_t decompress_us = 0;
    std::string decompressed_output_hash;
};
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Results/schema/result_json.h
#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {
std::string to_json(const GraphicsResult& result);
std::string to_json(const CompressionResult& result);
}  // namespace benchmark
```

**Step 4: Run the focused test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R result_json_test --output-on-failure`
Expected: PASS with serialized JSON containing required fields.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Results/schema/result_types.h PSO-compile-test/Test/Results/schema/result_types.cpp PSO-compile-test/Test/Results/schema/result_json.h PSO-compile-test/Test/Results/schema/result_json.cpp PSO-compile-test/Test/Results/tests/result_json_test.cpp
git commit -m "feat: add benchmark result schema"
```

### Task 3: Add workload manifests for Unreal-style shader tiers and compression baselines

**Files:**
- Create: `PSO-compile-test/Test/Shaders/manifests/shader_workload.h`
- Create: `PSO-compile-test/Test/Shaders/manifests/shader_workload.cpp`
- Create: `PSO-compile-test/Test/Compression/baseline_manifest.h`
- Create: `PSO-compile-test/Test/Compression/baseline_manifest.cpp`
- Create: `PSO-compile-test/Test/Shaders/tests/shader_workload_test.cpp`
- Create: `PSO-compile-test/Test/Compression/tests/baseline_manifest_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Shaders/tests/shader_workload_test.cpp`
- Test: `PSO-compile-test/Test/Compression/tests/baseline_manifest_test.cpp`

**Step 1: Write the failing shader workload test**

```cpp
// PSO-compile-test/Test/Shaders/tests/shader_workload_test.cpp
#include "Shaders/manifests/shader_workload.h"
#include "TestSupport/assert.h"

int main() {
    const auto workload = benchmark::make_base_pbr_workload();
    test_support::expect_true(workload.tier == "S1_BasePBR", "tier");
    test_support::expect_true(workload.permutation_count > 0, "permutations");
    return test_support::finish();
}
```

**Step 2: Run the tests to verify they fail**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R "shader_workload_test|baseline_manifest_test" --output-on-failure`
Expected: FAIL because the manifest types and helper constructors do not exist.

**Step 3: Write the minimal workload and baseline manifest types**

```cpp
// PSO-compile-test/Test/Shaders/manifests/shader_workload.h
#pragma once

#include <string>
#include <vector>

namespace benchmark {
struct ShaderWorkload {
    std::string tier;
    int texture_sample_count = 0;
    int static_switch_count = 0;
    int permutation_count = 0;
};

ShaderWorkload make_base_pbr_workload();
ShaderWorkload make_permutation_stress_workload();
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Compression/baseline_manifest.h
#pragma once

#include <string>

namespace benchmark {
struct CompressionBaselineManifest {
    std::string algorithm;
    std::string version;
    std::string baseline_package_id;
    std::string compressed_hash;
    std::string decompressed_hash;
};

CompressionBaselineManifest make_default_zstd_baseline();
}  // namespace benchmark
```

**Step 4: Run the tests to verify they pass**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R "shader_workload_test|baseline_manifest_test" --output-on-failure`
Expected: PASS with one test executable per manifest type.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Shaders/manifests/shader_workload.h PSO-compile-test/Test/Shaders/manifests/shader_workload.cpp PSO-compile-test/Test/Compression/baseline_manifest.h PSO-compile-test/Test/Compression/baseline_manifest.cpp PSO-compile-test/Test/Shaders/tests/shader_workload_test.cpp PSO-compile-test/Test/Compression/tests/baseline_manifest_test.cpp
git commit -m "feat: define shader and compression manifests"
```

### Task 4: Add the benchmark runner core and per-mode case scheduling

**Files:**
- Create: `PSO-compile-test/Test/Runner/benchmark_case.h`
- Create: `PSO-compile-test/Test/Runner/benchmark_runner.h`
- Create: `PSO-compile-test/Test/Runner/benchmark_runner.cpp`
- Create: `PSO-compile-test/Test/Runner/tests/benchmark_runner_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Runner/tests/benchmark_runner_test.cpp`

**Step 1: Write the failing runner scheduling test**

```cpp
// PSO-compile-test/Test/Runner/tests/benchmark_runner_test.cpp
#include "Runner/benchmark_runner.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("android_mesa_vulkan");
    test_support::expect_true(!plan.empty(), "mode has cases");
    test_support::expect_true(plan.front().driver_mode == "Mesa", "driver mode");
    return test_support::finish();
}
```

**Step 2: Run the test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R benchmark_runner_test --output-on-failure`
Expected: FAIL because runner classes do not exist.

**Step 3: Write the minimal runner implementation**

```cpp
// PSO-compile-test/Test/Runner/benchmark_case.h
#pragma once

#include <string>

namespace benchmark {
struct BenchmarkCase {
    std::string mode;
    std::string api;
    std::string driver_mode;
    std::string workload_tier;
};
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Runner/benchmark_runner.h
#pragma once

#include <string>
#include <vector>

#include "Runner/benchmark_case.h"

namespace benchmark {
class BenchmarkRunner {
public:
    std::vector<BenchmarkCase> plan_for_mode(const std::string& mode) const;
};
}  // namespace benchmark
```

**Step 4: Run the test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R benchmark_runner_test --output-on-failure`
Expected: PASS with at least one planned case for each supported mode.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Runner/benchmark_case.h PSO-compile-test/Test/Runner/benchmark_runner.h PSO-compile-test/Test/Runner/benchmark_runner.cpp PSO-compile-test/Test/Runner/tests/benchmark_runner_test.cpp
git commit -m "feat: add benchmark runner planning core"
```

### Task 5: Add compression baseline generation and mobile replay adapters

**Files:**
- Create: `PSO-compile-test/Test/Compression/compression_job.h`
- Create: `PSO-compile-test/Test/Compression/compression_job.cpp`
- Create: `PSO-compile-test/Test/Compression/decompression_replay.h`
- Create: `PSO-compile-test/Test/Compression/decompression_replay.cpp`
- Create: `PSO-compile-test/Test/Compression/tests/decompression_replay_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Compression/tests/decompression_replay_test.cpp`

**Step 1: Write the failing decompression replay test**

```cpp
// PSO-compile-test/Test/Compression/tests/decompression_replay_test.cpp
#include "Compression/decompression_replay.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::CompressionBaselineManifest baseline = benchmark::make_default_zstd_baseline();
    const auto result = benchmark::simulate_replay(baseline, 4096);
    test_support::expect_true(result.baseline_package_id == baseline.baseline_package_id, "baseline linked");
    test_support::expect_true(result.decompress_us >= 0, "timing emitted");
    return test_support::finish();
}
```

**Step 2: Run the test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R decompression_replay_test --output-on-failure`
Expected: FAIL because the replay adapter does not exist.

**Step 3: Write the minimal baseline and replay implementation**

```cpp
// PSO-compile-test/Test/Compression/decompression_replay.h
#pragma once

#include "Compression/baseline_manifest.h"
#include "Results/schema/result_types.h"

namespace benchmark {
CompressionResult simulate_replay(const CompressionBaselineManifest& baseline, int payload_size);
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Compression/decompression_replay.cpp
#include "Compression/decompression_replay.h"

namespace benchmark {
CompressionResult simulate_replay(const CompressionBaselineManifest& baseline, int) {
    CompressionResult result{};
    result.platform = "Simulated";
    result.algorithm = baseline.algorithm;
    result.baseline_package_id = baseline.baseline_package_id;
    result.decompressed_output_hash = baseline.decompressed_hash;
    result.decompress_us = 1;
    return result;
}
}  // namespace benchmark
```

**Step 4: Run the test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R decompression_replay_test --output-on-failure`
Expected: PASS with a replay result carrying baseline identity and timing.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Compression/compression_job.h PSO-compile-test/Test/Compression/compression_job.cpp PSO-compile-test/Test/Compression/decompression_replay.h PSO-compile-test/Test/Compression/decompression_replay.cpp PSO-compile-test/Test/Compression/tests/decompression_replay_test.cpp
git commit -m "feat: add compression baseline and replay adapters"
```

### Task 6: Add graphics timing interfaces for Mesa and native backends

**Files:**
- Create: `PSO-compile-test/Test/Graphics/common/graphics_backend.h`
- Create: `PSO-compile-test/Test/Graphics/common/timing_scope.h`
- Create: `PSO-compile-test/Test/Graphics/mesa/mesa_stage_collector.h`
- Create: `PSO-compile-test/Test/Graphics/native/native_backend_stub.h`
- Create: `PSO-compile-test/Test/Graphics/tests/graphics_backend_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Graphics/tests/graphics_backend_test.cpp`

**Step 1: Write the failing backend contract test**

```cpp
// PSO-compile-test/Test/Graphics/tests/graphics_backend_test.cpp
#include "Graphics/common/graphics_backend.h"
#include "TestSupport/assert.h"

int main() {
    const auto summary = benchmark::make_stub_native_backend().run_compile_case("S1_BasePBR");
    test_support::expect_true(summary.total_us >= 0, "timing emitted");
    test_support::expect_true(summary.api == "Stub", "api name");
    return test_support::finish();
}
```

**Step 2: Run the test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R graphics_backend_test --output-on-failure`
Expected: FAIL because the backend interface and stub do not exist.

**Step 3: Write the minimal backend contract**

```cpp
// PSO-compile-test/Test/Graphics/common/graphics_backend.h
#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {
class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;
    virtual GraphicsResult run_compile_case(const std::string& workload_tier) = 0;
};

GraphicsBackend& make_stub_native_backend();
}  // namespace benchmark
```

**Step 4: Run the test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R graphics_backend_test --output-on-failure`
Expected: PASS with the stub backend returning a valid result object.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Graphics/common/graphics_backend.h PSO-compile-test/Test/Graphics/common/timing_scope.h PSO-compile-test/Test/Graphics/mesa/mesa_stage_collector.h PSO-compile-test/Test/Graphics/native/native_backend_stub.h PSO-compile-test/Test/Graphics/tests/graphics_backend_test.cpp
git commit -m "feat: add graphics backend timing contracts"
```

### Task 7: Add Mesa trace ingestion and instrumentation metadata support

**Files:**
- Create: `PSO-compile-test/Test/Mesa/adapters/mesa_trace_event.h`
- Create: `PSO-compile-test/Test/Mesa/adapters/mesa_trace_parser.h`
- Create: `PSO-compile-test/Test/Mesa/adapters/mesa_trace_parser.cpp`
- Create: `PSO-compile-test/Test/Mesa/patches/README.md`
- Create: `PSO-compile-test/Test/Mesa/tests/mesa_trace_parser_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Mesa/tests/mesa_trace_parser_test.cpp`

**Step 1: Write the failing Mesa trace parser test**

```cpp
// PSO-compile-test/Test/Mesa/tests/mesa_trace_parser_test.cpp
#include "Mesa/adapters/mesa_trace_parser.h"
#include "TestSupport/assert.h"

int main() {
    const auto events = benchmark::parse_mesa_trace_lines({
        "stage=frontend duration_us=12",
        "stage=backend duration_us=20",
    });
    test_support::expect_true(events.size() == 2, "event count");
    test_support::expect_true(events.front().name == "frontend", "stage name");
    return test_support::finish();
}
```

**Step 2: Run the test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R mesa_trace_parser_test --output-on-failure`
Expected: FAIL because the parser and event types do not exist.

**Step 3: Write the minimal parser and patch README**

```cpp
// PSO-compile-test/Test/Mesa/adapters/mesa_trace_event.h
#pragma once

#include <cstdint>
#include <string>

namespace benchmark {
struct MesaTraceEvent {
    std::string name;
    std::int64_t duration_us = 0;
};
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Mesa/adapters/mesa_trace_parser.h
#pragma once

#include <string>
#include <vector>

#include "Mesa/adapters/mesa_trace_event.h"

namespace benchmark {
std::vector<MesaTraceEvent> parse_mesa_trace_lines(const std::vector<std::string>& lines);
}  // namespace benchmark
```

**Step 4: Run the test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R mesa_trace_parser_test --output-on-failure`
Expected: PASS with parsed stage names and durations.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Mesa/adapters/mesa_trace_event.h PSO-compile-test/Test/Mesa/adapters/mesa_trace_parser.h PSO-compile-test/Test/Mesa/adapters/mesa_trace_parser.cpp PSO-compile-test/Test/Mesa/patches/README.md PSO-compile-test/Test/Mesa/tests/mesa_trace_parser_test.cpp
git commit -m "feat: add Mesa trace ingestion support"
```

### Task 8: Add Android platform shell and mode selection bridge

**Files:**
- Create: `PSO-compile-test/Test/Platform/android/CMakeLists.txt`
- Create: `PSO-compile-test/Test/Platform/android/AndroidManifest.xml`
- Create: `PSO-compile-test/Test/Platform/android/MainActivity.kt`
- Create: `PSO-compile-test/Test/Platform/android/native_entry.cpp`
- Create: `PSO-compile-test/Test/Platform/android/tests/android_mode_bridge_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Platform/android/tests/android_mode_bridge_test.cpp`

**Step 1: Write the failing Android mode bridge test**

```cpp
// PSO-compile-test/Test/Platform/android/tests/android_mode_bridge_test.cpp
#include "Runner/benchmark_runner.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("android_native_opengl");
    test_support::expect_true(!plan.empty(), "android opengl mode available");
    return test_support::finish();
}
```

**Step 2: Run the Android-focused test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R android_mode_bridge_test --output-on-failure`
Expected: FAIL because Android shell files and mode wiring do not exist.

**Step 3: Write the minimal Android shell bridge**

```cpp
// PSO-compile-test/Test/Platform/android/native_entry.cpp
#include "Runner/benchmark_runner.h"

extern "C" const char* benchmark_default_mode() {
    static const char* kMode = "android_native_opengl";
    return kMode;
}
```

```kotlin
// PSO-compile-test/Test/Platform/android/MainActivity.kt
package com.example.psobenchmark

class MainActivity
```

**Step 4: Run the Android-focused test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R android_mode_bridge_test --output-on-failure`
Expected: PASS with the runner exposing Android modes.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Platform/android/CMakeLists.txt PSO-compile-test/Test/Platform/android/AndroidManifest.xml PSO-compile-test/Test/Platform/android/MainActivity.kt PSO-compile-test/Test/Platform/android/native_entry.cpp PSO-compile-test/Test/Platform/android/tests/android_mode_bridge_test.cpp
git commit -m "feat: add Android benchmark shell"
```

### Task 9: Add iOS Metal shell and replay bridge

**Files:**
- Create: `PSO-compile-test/Test/Platform/ios/CMakeLists.txt`
- Create: `PSO-compile-test/Test/Platform/ios/AppDelegate.mm`
- Create: `PSO-compile-test/Test/Platform/ios/MetalBenchmarkViewController.mm`
- Create: `PSO-compile-test/Test/Platform/ios/native_entry.mm`
- Create: `PSO-compile-test/Test/Platform/ios/tests/ios_mode_bridge_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Platform/ios/tests/ios_mode_bridge_test.cpp`

**Step 1: Write the failing iOS mode bridge test**

```cpp
// PSO-compile-test/Test/Platform/ios/tests/ios_mode_bridge_test.cpp
#include "Runner/benchmark_runner.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("ios_native_metal");
    test_support::expect_true(!plan.empty(), "ios metal mode available");
    return test_support::finish();
}
```

**Step 2: Run the iOS-focused test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R ios_mode_bridge_test --output-on-failure`
Expected: FAIL because iOS shell files and mode wiring do not exist.

**Step 3: Write the minimal iOS shell bridge**

```objective-c++
// PSO-compile-test/Test/Platform/ios/native_entry.mm
extern "C" const char* benchmark_default_ios_mode() {
    return "ios_native_metal";
}
```

```objective-c++
// PSO-compile-test/Test/Platform/ios/AppDelegate.mm
int main() {
    return 0;
}
```

**Step 4: Run the iOS-focused test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R ios_mode_bridge_test --output-on-failure`
Expected: PASS with the runner exposing the iOS Metal mode.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Platform/ios/CMakeLists.txt PSO-compile-test/Test/Platform/ios/AppDelegate.mm PSO-compile-test/Test/Platform/ios/MetalBenchmarkViewController.mm PSO-compile-test/Test/Platform/ios/native_entry.mm PSO-compile-test/Test/Platform/ios/tests/ios_mode_bridge_test.cpp
git commit -m "feat: add iOS Metal benchmark shell"
```

### Task 10: Add exporter and baseline package outputs for operator-readable reports

**Files:**
- Create: `PSO-compile-test/Test/Results/exporters/json_exporter.h`
- Create: `PSO-compile-test/Test/Results/exporters/json_exporter.cpp`
- Create: `PSO-compile-test/Test/Results/exporters/csv_exporter.h`
- Create: `PSO-compile-test/Test/Results/exporters/csv_exporter.cpp`
- Create: `PSO-compile-test/Test/Results/tests/exporter_test.cpp`
- Modify: `PSO-compile-test/Test/CMakeLists.txt`
- Test: `PSO-compile-test/Test/Results/tests/exporter_test.cpp`

**Step 1: Write the failing exporter test**

```cpp
// PSO-compile-test/Test/Results/tests/exporter_test.cpp
#include "Results/exporters/json_exporter.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::GraphicsResult result{};
    result.platform = "Android";
    result.api = "OpenGL";
    const auto json = benchmark::export_graphics_json(result);
    test_support::expect_true(json.find("OpenGL") != std::string::npos, "json export");
    return test_support::finish();
}
```

**Step 2: Run the exporter test to verify it fails**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R exporter_test --output-on-failure`
Expected: FAIL because exporter files do not exist.

**Step 3: Write the minimal exporters**

```cpp
// PSO-compile-test/Test/Results/exporters/json_exporter.h
#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {
std::string export_graphics_json(const GraphicsResult& result);
std::string export_compression_json(const CompressionResult& result);
}  // namespace benchmark
```

```cpp
// PSO-compile-test/Test/Results/exporters/csv_exporter.h
#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {
std::string export_graphics_csv_row(const GraphicsResult& result);
std::string export_compression_csv_row(const CompressionResult& result);
}  // namespace benchmark
```

**Step 4: Run the exporter test to verify it passes**

Run: `cmake --build build/pso-benchmark && ctest --test-dir build/pso-benchmark -R exporter_test --output-on-failure`
Expected: PASS with JSON and CSV rows containing core fields.

**Step 5: Commit**

```bash
git add PSO-compile-test/Test/CMakeLists.txt PSO-compile-test/Test/Results/exporters/json_exporter.h PSO-compile-test/Test/Results/exporters/json_exporter.cpp PSO-compile-test/Test/Results/exporters/csv_exporter.h PSO-compile-test/Test/Results/exporters/csv_exporter.cpp PSO-compile-test/Test/Results/tests/exporter_test.cpp
git commit -m "feat: add benchmark result exporters"
```

## Notes for the Implementer

- Use `@test-driven-development` before touching implementation code in each task.
- Use `@verification-before-completion` before claiming any task is done.
- Keep all benchmark logic under `PSO-compile-test/Test`.
- Do not promise full Mesa instrumentation depth in the first pass; land stable stage boundaries first.
- Keep Windows graphics work out of the critical path.
