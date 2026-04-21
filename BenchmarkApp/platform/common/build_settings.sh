#!/bin/bash

benchmark_build_root() {
  local app_root="$1"
  echo "${BUILD_ROOT:-$app_root/../build/BenchmarkApp}"
}

benchmark_build_type() {
  echo "${BENCHMARK_BUILD_TYPE:-${CMAKE_BUILD_TYPE:-Release}}"
}

benchmark_build_jobs() {
  echo "${BENCHMARK_BUILD_JOBS:-${CMAKE_BUILD_PARALLEL_LEVEL:-4}}"
}

benchmark_apple_configuration() {
  echo "${BENCHMARK_IOS_CONFIGURATION:-${BENCHMARK_APPLE_CONFIGURATION:-${CONFIGURATION:-$(benchmark_build_type)}}}"
}

benchmark_ios_app_path() {
  local app_root="$1"
  local mode="${2:-device}"
  local build_root
  local sdk
  local build_dir

  build_root=$(benchmark_build_root "$app_root")

  case "$mode" in
    device)
      sdk="iphoneos"
      build_dir="ios-device"
      ;;
    simulator|sim)
      sdk="iphonesimulator"
      build_dir="ios-sim"
      ;;
    *)
      return 1
      ;;
  esac

  echo "$build_root/$build_dir/platform/ios/$(benchmark_apple_configuration)-$sdk/PSOBenchmarkApp.app"
}

benchmark_windows_configuration() {
  echo "${BENCHMARK_WINDOWS_CONFIGURATION:-${CONFIGURATION:-$(benchmark_build_type)}}"
}

benchmark_first_existing_path() {
  for candidate in "$@"; do
    if [[ -e "$candidate" ]]; then
      echo "$candidate"
      return 0
    fi
  done

  return 1
}

benchmark_windows_binary_candidates() {
  local build_root="$1"
  local config="${2:-$(benchmark_windows_configuration)}"

  printf '%s\n' \
    "$build_root/windows/platform/windows/compression_benchmark_cli.exe" \
    "$build_root/windows/platform/windows/$config/compression_benchmark_cli.exe" \
    "$build_root/windows/$config/platform/windows/compression_benchmark_cli.exe"
}
