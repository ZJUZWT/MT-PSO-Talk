#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

BUILD_ROOT="$TMP_DIR/build/BenchmarkApp" \
  "$APP_ROOT/platform/ios/build_ios.sh" simulator >/dev/null

APP_PATH="$TMP_DIR/build/BenchmarkApp/ios-sim/platform/ios/Release-iphonesimulator/PSOBenchmarkApp.app"
INFO_PLIST="$APP_PATH/Info.plist"

if [[ ! -f "$INFO_PLIST" ]]; then
  echo "Expected simulator build to produce $INFO_PLIST"
  exit 1
fi

if [[ "$(/usr/bin/plutil -extract CFBundleName raw -o - "$INFO_PLIST" 2>/dev/null || true)" != "PSO Benchmark" ]]; then
  echo "Expected simulator bundle Info.plist to include CFBundleName=PSO Benchmark"
  exit 1
fi

if [[ "$(/usr/bin/plutil -extract CFBundleIdentifier raw -o - "$INFO_PLIST" 2>/dev/null || true)" != "com.pso.benchmark" ]]; then
  echo "Expected simulator bundle Info.plist to include CFBundleIdentifier=com.pso.benchmark"
  exit 1
fi
