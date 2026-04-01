# Cross-Platform PSO and Compression Benchmark Design

## Summary

This document defines the initial design for a benchmark-focused project under `PSO-compile-test` that measures:

- PSO-related graphics API costs across Android and iOS
- Mesa-instrumented internal stage timing for Android OpenGL and Vulkan
- Native-driver API timing for Android OpenGL/Vulkan and iOS Metal
- Compression behavior with Windows-generated compression baselines and mobile-device decompression replay

The repository remains test-driven in structure: all benchmark logic, runners, assets, adapters, and result handling live under a `Test` hierarchy.

## Goals

- Support Android benchmark execution for:
  - Mesa OpenGL
  - Mesa Vulkan
  - Native OpenGL
  - Native Vulkan
  - Compression decompression replay
- Support iOS benchmark execution for:
  - Native Metal
  - Compression decompression replay
- Support Windows benchmark execution for:
  - Compression baseline generation
  - Compression and decompression verification
- Produce directly comparable output for the same shader workloads and compression assets across platforms
- Expose Mesa internal stage timing for PSO-related work on Android Mesa paths

## Non-Goals for V1

- Full Windows graphics API benchmark support
- macOS benchmark delivery
- iOS Mesa support
- Native-driver internal stage breakdown beyond public graphics API boundaries
- Exhaustive per-pass Mesa compiler instrumentation from day one

## Platform Matrix

| Platform | API / Area | Driver Mode | V1 | Output |
| --- | --- | --- | --- | --- |
| Android | OpenGL | Mesa | Yes | API timing + Mesa stage timing |
| Android | Vulkan | Mesa | Yes | API timing + Mesa stage timing |
| Android | OpenGL | Native | Yes | API timing |
| Android | Vulkan | Native | Yes | API timing |
| Android | Compression | Native CPU | Yes | Decompression timing + correctness |
| iOS | Metal | Native | Yes | API timing |
| iOS | Compression | Native CPU | Yes | Decompression timing + correctness |
| Windows | Compression | Native CPU | Yes | Compression baseline + decompression timing |

Windows graphics support remains a low-priority extension path and does not enter V1 scope.

## Measurement Model

### Graphics Timing Layers

All graphics backends emit a unified API-level timing layer:

- shader compile total
- link or pipeline creation total
- first-draw-ready total
- cold-cache and warm-cache runs

Mesa Android paths add an internal stage timing layer driven by a maintained Mesa instrumentation patch set. V1 instrumentation targets stable compiler and pipeline boundaries instead of every individual optimization pass.

Expected internal stage buckets include:

- frontend ingest
- IR conversion and lowering
- optimization bucket
- backend code generation
- link or pipeline finalization

The exact stage names will follow real Mesa code boundaries rather than pre-baked labels.

### PSO Semantics by API

OpenGL does not expose an explicit PSO object, so benchmark units are defined as:

- shader stage compile
- program link
- first-use variant or materialization work

Vulkan benchmark units are defined around:

- `vkCreateShaderModule`
- `vkCreatePipelineLayout`
- `vkCreateGraphicsPipelines`
- cache hit and miss behavior

Metal benchmark units are defined around:

- library creation
- function lookup
- render pipeline state creation
- first-draw-ready

## Shader Workload Model

Shader assets are organized around Unreal-style workload classes rather than ad hoc long shader files.

### Workload Tiers

- `S1 BasePBR`
  - standard PBR shading
  - normal, roughness, metallic usage
  - low-to-medium branching
- `S2 MaterialLayered`
  - layered materials
  - more texture sampling
  - static feature switches
- `S3 FeatureHeavy`
  - heavier material features such as clear coat, anisotropy, subsurface-like branches, and virtual-texture-like paths
  - designed to stress optimization and backend compilation
- `S4 PermutationStress`
  - many macro and state permutations
  - closer to Unreal pipeline explosion behavior than a single oversized shader

### Workload Dimensions

Each test case records two dimensions:

- `ShaderComplexity`
  - instruction scale
  - texture sample count
  - control-flow depth
  - resource binding count
  - stage count
- `PermutationComplexity`
  - macro count
  - static switch count
  - render-state combination count
  - layout and descriptor variation count

## Compression Design

Compression is split between baseline generation and mobile replay.

### Windows Responsibilities

Windows is the baseline generation machine for compression:

- generate compressed outputs
- record compressed size
- record compression ratio
- record compression time
- record decompression time
- record compressed-output hash
- record decompressed-output hash

### Android and iOS Responsibilities

Android and iOS only replay decompression:

- consume Windows-generated compression baseline artifacts
- measure decompression time and throughput
- validate decompressed payload hash
- report compatibility and correctness

This avoids mixing mobile CPU variability into compression-generation comparisons while still measuring real device-side runtime cost.

### Compression Result Categories

Each compression result records:

- algorithm
- algorithm version
- level or profile
- input identifier
- input size
- compressed size
- compression ratio
- compress time where applicable
- decompress time
- compressed-output hash where applicable
- decompressed-output hash
- baseline package identifier
- determinism or compatibility status

## Result Schema

JSON is the primary result format. CSV is generated as an export view.

### Common Fields

- platform
- device model
- SoC or CPU/GPU identity
- OS version
- test mode
- API
- driver mode
- benchmark case
- iteration index
- cold or warm flag
- success status
- error classification

### Graphics-Specific Fields

- shader compile total microseconds
- link or pipeline create microseconds
- first-draw-ready microseconds
- total microseconds
- Mesa version or commit where applicable
- stage breakdown list for Mesa runs

### Compression-Specific Fields

- algorithm
- version
- level
- input size
- compressed size
- compression ratio
- compress microseconds where applicable
- decompress microseconds
- throughput
- compressed-output hash where applicable
- decompressed-output hash
- baseline package identifier

## Result Status Semantics

Every run must explicitly emit one of:

- `passed`
- `unsupported`
- `setup_error`
- `runtime_error`

This prevents unsupported or failed runs from silently appearing as zero-cost data points.

## Repository Layout

All benchmark code stays in `Test` under `PSO-compile-test`.

Proposed layout:

```text
PSO-compile-test/
  Test/
    Runner/
    Graphics/
      common/
      mesa/
        opengl/
        vulkan/
      native/
        opengl/
        vulkan/
        metal/
    Compression/
    Shaders/
      generators/
      assets/
      manifests/
    Mesa/
      patches/
      adapters/
      trace/
    Platform/
      android/
      ios/
      windows/
    Results/
      schema/
      exporters/
      baselines/
```

## Execution Strategy

### Android

Android uses a unified benchmark app with mode selection:

- Mesa OpenGL
- Mesa Vulkan
- Native OpenGL
- Native Vulkan
- Compression replay

V1 should target a validated Android device family rather than claiming universal Mesa deployment on arbitrary commercial devices.

### iOS

iOS uses a native Metal benchmark runner plus compression replay.

No Mesa path is planned for V1.

### Windows

Windows provides the compression baseline generator and verification runner.

## Risks and Constraints

- Mesa internal instrumentation requires maintaining a custom patch set
- Android Mesa deployment can be device-family-sensitive
- iOS provides only API-level timing, not Mesa-like internal stage breakdown
- Unreal-like shader complexity must be modeled carefully so cases stay comparable across OpenGL, Vulkan, and Metal

## V1 Deliverables

- Android benchmark support for Mesa and Native OpenGL/Vulkan
- iOS benchmark support for Native Metal
- Windows compression baseline generation
- Android and iOS decompression replay
- Unified JSON schema and export pipeline
- Initial Unreal-style shader workload set
- Mesa stage timing for Android Mesa paths
