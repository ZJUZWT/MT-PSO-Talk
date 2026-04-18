#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/build_settings.sh"

BUILD_ROOT=$(benchmark_build_root "$APP_ROOT")
APP_PATH="${1:-$(benchmark_ios_app_path "$APP_ROOT" device)}"
IPA_PATH="${2:-$BUILD_ROOT/ios-device/PSOBenchmarkApp.ipa}"

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
