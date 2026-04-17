#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$SCRIPT_DIR/android_ndk_utils.sh"

BUILD_ROOT="${BUILD_ROOT:-$APP_ROOT/../build/BenchmarkApp}"
ABI="${ANDROID_ABI:-arm64-v8a}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
ANDROID_SDK_ROOT_VALUE="${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}"

find_java_home() {
  local candidates=()

  if [[ -n "${JAVA_HOME:-}" ]]; then
    candidates+=("$JAVA_HOME")
  fi

  candidates+=(
    "/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home"
    "/opt/homebrew/opt/openjdk/libexec/openjdk.jdk/Contents/Home"
    "/Library/Java/JavaVirtualMachines/openjdk-17.jdk/Contents/Home"
  )

  for candidate in "${candidates[@]}"; do
    if [[ -x "$candidate/bin/java" ]]; then
      echo "$candidate"
      return 0
    fi
  done

  return 1
}

find_ndk_root() {
  local candidates=()
  local auto_detected=""

  if [[ -n "${ANDROID_NDK_HOME:-}" ]]; then
    candidates+=("$ANDROID_NDK_HOME")
  fi
  if [[ -n "${ANDROID_NDK:-}" ]]; then
    candidates+=("$ANDROID_NDK")
  fi
  candidates+=(
    "$ANDROID_SDK_ROOT_VALUE/ndk-bundle"
  )

  if auto_detected=$(benchmark_find_preferred_ndk_root "$ANDROID_SDK_ROOT_VALUE"); then
    candidates+=("$auto_detected")
  fi

  if [[ -d "$ANDROID_SDK_ROOT_VALUE/ndk" ]]; then
    while IFS= read -r dir; do
      candidates+=("$dir")
    done < <(find "$ANDROID_SDK_ROOT_VALUE/ndk" -maxdepth 1 -mindepth 1 -type d | sort -Vr)
  fi

  for candidate in "${candidates[@]}"; do
    if [[ -f "$candidate/build/cmake/android.toolchain.cmake" ]]; then
      echo "$candidate"
      return 0
    fi
  done

  return 1
}

if [[ -z "${JAVA_HOME:-}" ]]; then
  if JAVA_HOME=$(find_java_home); then
    export JAVA_HOME
    export PATH="$JAVA_HOME/bin:$PATH"
  fi
fi

if ! command -v java >/dev/null 2>&1; then
  echo "Java runtime not found."
  echo "Install a JDK or set JAVA_HOME before running this script."
  exit 1
fi

NDK_ROOT="${ANDROID_NDK_ROOT:-}"
if [[ -z "$NDK_ROOT" ]]; then
  if ! NDK_ROOT=$(find_ndk_root); then
    echo "Android NDK not found."
    echo "Run BenchmarkApp/platform/android/setup_android_sdk.sh first,"
    echo "or set ANDROID_NDK_HOME, ANDROID_NDK, or ANDROID_NDK_ROOT before running this script."
    exit 1
  fi
fi

BUILD_DIR="$BUILD_ROOT/android-$ABI"

echo "Using Java runtime: ${JAVA_HOME:-$(command -v java)}"
echo "Using Android NDK: $NDK_ROOT"

cmake -S "$APP_ROOT" -B "$BUILD_DIR" \
  -DANDROID=ON \
  -DCMAKE_TOOLCHAIN_FILE="$NDK_ROOT/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM="${ANDROID_PLATFORM:-android-24}" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

cmake --build "$BUILD_DIR" --target pso_benchmark -j4

echo "Android binary built at: $BUILD_DIR/platform/android/pso_benchmark"
