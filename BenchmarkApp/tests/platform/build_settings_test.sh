#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/build_settings.sh"

unset BUILD_ROOT BENCHMARK_BUILD_TYPE CMAKE_BUILD_TYPE BENCHMARK_BUILD_JOBS CMAKE_BUILD_PARALLEL_LEVEL
unset BENCHMARK_IOS_CONFIGURATION BENCHMARK_APPLE_CONFIGURATION CONFIGURATION BENCHMARK_WINDOWS_CONFIGURATION

default_root=$(benchmark_build_root "/tmp/example-app")
if [[ "$default_root" != "/tmp/example-app/../build/BenchmarkApp" ]]; then
  echo "Unexpected default build root: $default_root"
  exit 1
fi

if [[ "$(benchmark_build_type)" != "Release" ]]; then
  echo "Expected default build type Release"
  exit 1
fi

if [[ "$(benchmark_build_jobs)" != "4" ]]; then
  echo "Expected default build jobs 4"
  exit 1
fi

BENCHMARK_BUILD_TYPE=RelWithDebInfo
if [[ "$(benchmark_build_type)" != "RelWithDebInfo" ]]; then
  echo "Expected BENCHMARK_BUILD_TYPE to override the default build type"
  exit 1
fi

BENCHMARK_BUILD_JOBS=12
if [[ "$(benchmark_build_jobs)" != "12" ]]; then
  echo "Expected BENCHMARK_BUILD_JOBS to override the default build jobs"
  exit 1
fi

CONFIGURATION=Debug
if [[ "$(benchmark_apple_configuration)" != "Debug" ]]; then
  echo "Expected CONFIGURATION to drive the default Apple configuration"
  exit 1
fi

BENCHMARK_IOS_CONFIGURATION=Profile
if [[ "$(benchmark_apple_configuration)" != "Profile" ]]; then
  echo "Expected BENCHMARK_IOS_CONFIGURATION to override the Apple configuration"
  exit 1
fi

BENCHMARK_WINDOWS_CONFIGURATION=Checked
if [[ "$(benchmark_windows_configuration)" != "Checked" ]]; then
  echo "Expected BENCHMARK_WINDOWS_CONFIGURATION to override the Windows configuration"
  exit 1
fi

windows_candidates=()
while IFS= read -r candidate; do
  windows_candidates+=("$candidate")
done < <(benchmark_windows_binary_candidates "/tmp/build-root" "Release")
if [[ "${#windows_candidates[@]}" -ne 3 ]]; then
  echo "Expected three Windows binary candidates"
  exit 1
fi

if [[ "${windows_candidates[1]}" != "/tmp/build-root/windows/platform/windows/Release/compression_benchmark_cli.exe" ]]; then
  echo "Unexpected multi-config Windows binary candidate: ${windows_candidates[1]}"
  exit 1
fi
