# Cross-Shell Benchmark Runners Design

**Context**

`BenchmarkApp` already ships one-command shell runners for macOS, Android, and iOS, plus a PowerShell runner for Windows. The remaining gap is tester convenience: Android and iOS release bundles do not yet include matching `ps1` launch-and-pull-data scripts.

**Goal**

Add PowerShell launchers for Android and iOS so the release bundle exposes both `sh` and `ps1` entry points where the platform tooling allows it.

**Decisions**

- Add `BenchmarkApp/platform/android/run_benchmark.ps1`.
- Add `BenchmarkApp/platform/ios/run_benchmark.ps1`.
- Keep the existing `sh` scripts unchanged for current flows.
- Update `assemble_release.sh` so `release/scripts/` includes `run_android.ps1` and `run_ios.ps1`.
- Document that Android `ps1` can run on any machine with `pwsh` + `adb`, while iOS `ps1` still requires macOS because it depends on `xcrun devicectl`.

**Testing**

- Extend the release assembly shell test to fail until both new `ps1` scripts are present in `release/scripts/`.
- After implementation, re-run the release assembly test and a fresh full `ctest` pass.

