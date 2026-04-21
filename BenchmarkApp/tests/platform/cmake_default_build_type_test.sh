#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

cmake -S "$APP_ROOT" -B "$TMP_DIR/build" >/dev/null

if ! rg -q '^CMAKE_BUILD_TYPE:STRING=Release$' "$TMP_DIR/build/CMakeCache.txt"; then
  echo "Expected single-config CMake configure to default to Release"
  echo "Actual CMAKE_BUILD_TYPE line:"
  rg '^CMAKE_BUILD_TYPE:STRING=' "$TMP_DIR/build/CMakeCache.txt" || true
  exit 1
fi
