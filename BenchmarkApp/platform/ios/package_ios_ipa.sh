#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

APP_PATH="${1:-$APP_ROOT/../build/BenchmarkApp/ios-device/platform/ios/Release-iphoneos/PSOBenchmarkApp.app}"
IPA_PATH="${2:-$APP_ROOT/../build/BenchmarkApp/ios-device/PSOBenchmarkApp.ipa}"

if [[ ! -d "$APP_PATH" ]]; then
  echo "iOS app bundle not found: $APP_PATH"
  exit 1
fi

PACKAGE_ROOT=$(mktemp -d)
trap 'rm -rf "$PACKAGE_ROOT"' EXIT

mkdir -p "$PACKAGE_ROOT/Payload"
cp -R "$APP_PATH" "$PACKAGE_ROOT/Payload/"
mkdir -p "$(dirname "$IPA_PATH")"

(
  cd "$PACKAGE_ROOT"
  rm -f "$IPA_PATH"
  zip -qry "$IPA_PATH" Payload
)

echo "iOS IPA packaged at: $IPA_PATH"
