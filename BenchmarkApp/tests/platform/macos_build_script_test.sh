#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

BUILD_ROOT="$TMP_DIR/build/BenchmarkApp" \
BENCHMARK_CONFIGURE_ONLY=1 \
  "$APP_ROOT/platform/macos/build_macos.sh" >/dev/null

if ! rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$TMP_DIR/build/BenchmarkApp/macos/CMakeCache.txt"; then
  echo "Expected build_macos.sh to configure a Release build by default"
  echo "Actual CMAKE_BUILD_TYPE line:"
  rg '^CMAKE_BUILD_TYPE:STRING=' "$TMP_DIR/build/BenchmarkApp/macos/CMakeCache.txt" || true
  exit 1
fi
