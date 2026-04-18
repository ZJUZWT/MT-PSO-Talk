#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/build_settings.sh"

BUILD_ROOT=$(benchmark_build_root "$APP_ROOT")
BUILD_DIR="$BUILD_ROOT/macos"
BUILD_TYPE=$(benchmark_build_type)
TARGET_NAME="${BENCHMARK_BUILD_TARGET:-benchmark_main}"
CONFIGURE_ONLY="${BENCHMARK_CONFIGURE_ONLY:-0}"
BUILD_JOBS=$(benchmark_build_jobs)

cmake -S "$APP_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

if [[ "$CONFIGURE_ONLY" == "1" ]]; then
  echo "Configured macOS benchmark build at: $BUILD_DIR"
  exit 0
fi

cmake --build "$BUILD_DIR" --target "$TARGET_NAME" -j"$BUILD_JOBS"

echo "macOS benchmark binary built at: $BUILD_DIR/lib/$TARGET_NAME"
