# Compression Benchmark App

`BenchmarkApp` is now a compression-first benchmark harness for measuring real compression and decompression performance across macOS, Windows, Android, and iOS.

## What It Runs

- Built-in compression algorithms, including `noop`, `zstd`, `lz4`, `zlib`, `snappy`, and `brotli`
- Optional Oodle `kraken` / `leviathan` / `mermaid` / `selkie` when a licensed Oodle SDK is supplied locally
- End-to-end compression and decompression timing
- JSON and CSV export for product benchmarking
- Multi-profile stress coverage by default: `pso_like`, `high_compressibility`, and `low_compressibility`
- Multiple payload sizes by default: `64KB`, `256KB`, and `1MB`
- Explicit roundtrip validation using both hash comparison and byte-for-byte equality

Graphics benchmarking code is still present in the repo, but the product default is compression-only. Pass `--graphics` only when you explicitly want the legacy mixed report.

## Oodle SDK Layout

This repository does not ship the proprietary Oodle SDK. To enable Oodle,
provide a licensed Unreal Engine `OodleDataCompression` SDK `2.9.12` in one of
these ways:

- Copy it into the placeholder tree under:

```text
BenchmarkApp/third_party/oodle/2.9.12
```

- Or point CMake at a private copy:

```bash
cmake -S BenchmarkApp -B build/benchmark \
  -DCMAKE_BUILD_TYPE=Release \
  -DBENCHMARK_OODLE_ROOT=/absolute/path/to/OodleDataCompression/Sdks/2.9.12
```

- Or set an environment variable before invoking any build script:

```bash
export BENCHMARK_OODLE_ROOT=/absolute/path/to/OodleDataCompression/Sdks/2.9.12
```

Expected layout:

```text
BenchmarkApp/third_party/oodle/2.9.12
├── include/oodle2.h
├── include/oodle2base.h
└── lib/...
```

Platform selection is automatic:

- macOS: `lib/Mac/liboo2coremac64.a`
- Windows: `lib/Win64/oo2core_win64.lib`
- Android: `lib/Android/<abi>/liboo2coreandroid.a`
- iOS: `lib/IOS/liboo2coreios.a`

If the SDK is missing, the project still builds, but Oodle algorithms are not
compiled into the benchmark.

## Build

```bash
BenchmarkApp/platform/macos/build_macos.sh
```

For generic single-config desktop builds, the project now defaults to `Release` when
`CMAKE_BUILD_TYPE` is omitted so cross-device benchmark runs do not accidentally mix
optimized and unoptimized binaries.

All benchmark build entry points now share the same defaults:

- `BUILD_ROOT`: defaults to `build/BenchmarkApp`
- `BENCHMARK_BUILD_TYPE`: defaults to `Release`
- `BENCHMARK_BUILD_JOBS`: defaults to `4`
- `BENCHMARK_IOS_CONFIGURATION`: optional override for iOS multi-config output paths
- `BENCHMARK_WINDOWS_CONFIGURATION`: optional override for Windows multi-config output paths

Manual desktop configure/build remains available:

```bash
cmake -S BenchmarkApp -B build/benchmark -DCMAKE_BUILD_TYPE=Release
cmake --build build/benchmark -j4
```

Desktop configure/build with a private Oodle SDK:

```bash
cmake -S BenchmarkApp -B build/benchmark \
  -DCMAKE_BUILD_TYPE=Release \
  -DBENCHMARK_OODLE_ROOT=/absolute/path/to/OodleDataCompression/Sdks/2.9.12
cmake --build build/benchmark -j4
```

Android and iOS build entry points also honor the same `BUILD_ROOT`,
`BENCHMARK_BUILD_TYPE`, and `BENCHMARK_BUILD_JOBS` defaults:

```bash
BenchmarkApp/platform/android/build_android.sh
IOS_DEVELOPMENT_TEAM=ABCDE12345 BenchmarkApp/platform/ios/build_ios.sh device
```

Windows build entry point:

```powershell
powershell -File BenchmarkApp\platform\windows\build_windows.ps1
```

## Run

Desktop:

```bash
./build/benchmark/lib/benchmark_main --json benchmark_report.json --csv compression_results.csv
```

Desktop with custom stress settings:

```bash
./build/benchmark/lib/benchmark_main \
  --compression-iterations 3 \
  --compression-warmup-iterations 1 \
  --payload-sizes 64k,256k,1m \
  --payload-profiles pso,high,low \
  --json benchmark_report.json \
  --csv compression_results.csv
```

macOS one-command runner:

```bash
BenchmarkApp/platform/macos/build_macos.sh
BenchmarkApp/platform/macos/run_benchmark.sh
```

Android:

```bash
BenchmarkApp/platform/android/build_android.sh
BenchmarkApp/platform/android/run_benchmark.sh
```

Android via PowerShell:

```powershell
pwsh BenchmarkApp/platform/android/run_benchmark.ps1
```

Android CLI binaries also accept:

```text
--compression-iterations N
--compression-warmup-iterations N
--payload-sizes 64k,256k,1m
--payload-profiles pso,high,low
```

Windows:

```bash
BenchmarkApp\platform\windows\build_windows.ps1
BenchmarkApp\\platform\\windows\\run_benchmark.ps1
```

iOS real device:

```bash
IOS_DEVELOPMENT_TEAM=ABCDE12345 IOS_BUNDLE_IDENTIFIER=com.example.psobenchmark BenchmarkApp/platform/ios/build_ios.sh device
BenchmarkApp/platform/ios/run_benchmark.sh
```

iOS real device via PowerShell on macOS:

```powershell
pwsh BenchmarkApp/platform/ios/run_benchmark.ps1
```

iOS simulator:

```bash
BenchmarkApp/platform/ios/build_ios.sh simulator
```

Optional iOS `.ipa` packaging:

```bash
BenchmarkApp/platform/ios/package_ios_ipa.sh
```

Assemble a fixed `release/` directory that overwrites previous contents:

```bash
BenchmarkApp/platform/release/assemble_release.sh
```

## Outputs

- `benchmark_results/<platform>/<timestamp>/benchmark_report.json`
- `benchmark_results/<platform>/<timestamp>/compression_results.csv`
- `benchmark_results/<platform>/<timestamp>/run_info.txt`
- `release/` with:
  - `packages/`
  - `scripts/`
  - `results/`
  - `manifest.txt`

## Notes

- Compression results use the active host platform label instead of the old `Simulated` placeholder.
- On supported Apple and Android devices, the benchmark collects real timing data from the running device.
- The committed `BenchmarkApp/third_party/oodle/` tree only contains placeholder docs and empty folders; actual Oodle SDK files remain gitignored and must be supplied locally.
- Cross-device performance comparisons must use `Release` builds on every platform. An
  unoptimized desktop build can make macOS appear dramatically slower than Android even
  when the hardware is faster.
- Every platform runner writes a `run_info.txt` summary that records the device name, run time, and source binary or app path.
- Compression results now record `payload_profile`, `iteration_index`, `input_hash`, `roundtrip_hash_match`, and `roundtrip_byte_match` in JSON and CSV output.
- The iOS build output is a `.app` bundle because `xcodebuild` and `xcrun devicectl device install app` use the `.app` directly. Use `package_ios_ipa.sh` only when you specifically need an `.ipa`.
- `assemble_release.sh` wipes and rebuilds the repository-level `release/` directory so the packaged binaries and wrapper scripts stay in one place.
- `run_benchmark.ps1` is available for Android and iOS. Android can run anywhere `pwsh` and `adb` are available; iOS PowerShell execution still requires macOS because it depends on `xcrun devicectl`.
- The heavier default stress run means iOS runner timeouts now default to `900` seconds instead of `300`.
