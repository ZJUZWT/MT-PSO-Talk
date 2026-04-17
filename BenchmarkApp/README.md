# Compression Benchmark App

`BenchmarkApp` is now a compression-first benchmark harness for measuring real compression and decompression performance across macOS, Windows, Android, and iOS.

## What It Runs

- Built-in and vendored compression algorithms, including `noop`, `zstd`, `lz4`, `zlib`, `snappy`, `brotli`, and Oodle `kraken` / `leviathan` / `mermaid` / `selkie`
- End-to-end compression and decompression timing
- JSON and CSV export for product benchmarking

Graphics benchmarking code is still present in the repo, but the product default is compression-only. Pass `--graphics` only when you explicitly want the legacy mixed report.

## Oodle SDK Layout

The project vendors Unreal Engine `OodleDataCompression` SDK `2.9.12` under:

```text
BenchmarkApp/third_party/oodle/2.9.12
```

Platform selection is automatic:

- macOS: `lib/Mac/liboo2coremac64.a`
- Windows: `lib/Win64/oo2core_win64.lib`
- Android: `lib/Android/<abi>/liboo2coreandroid.a`
- iOS: `lib/IOS/liboo2coreios.a`

## Build

```bash
cmake -S BenchmarkApp -B build/benchmark
cmake --build build/benchmark -j4
```

## Run

Desktop:

```bash
./build/benchmark/lib/benchmark_main --json benchmark_report.json --csv compression_results.csv
```

macOS one-command runner:

```bash
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

Windows:

```bash
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
- Every platform runner writes a `run_info.txt` summary that records the device name, run time, and source binary or app path.
- The iOS build output is a `.app` bundle because `xcodebuild` and `xcrun devicectl device install app` use the `.app` directly. Use `package_ios_ipa.sh` only when you specifically need an `.ipa`.
- `assemble_release.sh` wipes and rebuilds the repository-level `release/` directory so the packaged binaries and wrapper scripts stay in one place.
- `run_benchmark.ps1` is available for Android and iOS. Android can run anywhere `pwsh` and `adb` are available; iOS PowerShell execution still requires macOS because it depends on `xcrun devicectl`.
