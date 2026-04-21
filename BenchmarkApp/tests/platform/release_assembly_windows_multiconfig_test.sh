#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

BUILD_ROOT="$TMP_DIR/build/BenchmarkApp"
RELEASE_ROOT="$TMP_DIR/release"

mkdir -p "$BUILD_ROOT/macos/lib"
mkdir -p "$BUILD_ROOT/android-arm64-v8a/platform/android"
mkdir -p "$BUILD_ROOT/ios-device/platform/ios/Release-iphoneos/PSOBenchmarkApp.app"
mkdir -p "$BUILD_ROOT/windows/platform/windows/Release"

printf '#!/bin/bash\necho mac\n' > "$BUILD_ROOT/macos/lib/benchmark_main"
printf '#!/bin/bash\necho android\n' > "$BUILD_ROOT/android-arm64-v8a/platform/android/pso_benchmark"
printf 'plist' > "$BUILD_ROOT/ios-device/platform/ios/Release-iphoneos/PSOBenchmarkApp.app/Info.plist"
printf 'exe' > "$BUILD_ROOT/windows/platform/windows/Release/compression_benchmark_cli.exe"
chmod +x "$BUILD_ROOT/macos/lib/benchmark_main" "$BUILD_ROOT/android-arm64-v8a/platform/android/pso_benchmark"

BUILD_ROOT="$BUILD_ROOT" RELEASE_ROOT="$RELEASE_ROOT" \
  "$APP_ROOT/platform/release/assemble_release.sh"

if [[ ! -f "$RELEASE_ROOT/packages/windows/compression_benchmark_cli.exe" ]]; then
  echo "Expected Windows package to be copied from a multi-config Release directory"
  exit 1
fi

if ! rg -q '^windows: packages/windows/compression_benchmark_cli.exe$' "$RELEASE_ROOT/manifest.txt"; then
  echo "Expected Windows package entry in manifest"
  exit 1
fi
