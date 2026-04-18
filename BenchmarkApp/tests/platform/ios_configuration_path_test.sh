#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

BUILD_ROOT="$TMP_DIR/build/BenchmarkApp"
APP_PATH="$BUILD_ROOT/ios-device/platform/ios/Debug-iphoneos/PSOBenchmarkApp.app"
IPA_PATH="$TMP_DIR/output/PSOBenchmarkApp.ipa"

mkdir -p "$APP_PATH"
printf 'debug-plist' > "$APP_PATH/Info.plist"

BUILD_ROOT="$BUILD_ROOT" BENCHMARK_IOS_CONFIGURATION=Debug \
  "$APP_ROOT/platform/ios/package_ios_ipa.sh" "" "$IPA_PATH" >/dev/null

if [[ ! -f "$IPA_PATH" ]]; then
  echo "Expected package_ios_ipa.sh to resolve the iOS app path from BENCHMARK_IOS_CONFIGURATION"
  exit 1
fi

EXTRACT_DIR="$TMP_DIR/extracted"
mkdir -p "$EXTRACT_DIR"
unzip -q "$IPA_PATH" -d "$EXTRACT_DIR"

if ! rg -q '^debug-plist$' "$EXTRACT_DIR/Payload/PSOBenchmarkApp.app/Info.plist"; then
  echo "Expected packaged IPA to contain the Debug configuration app bundle"
  exit 1
fi
