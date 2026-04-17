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
mkdir -p "$BUILD_ROOT/windows/platform/windows"

printf '#!/bin/bash\necho mac\n' > "$BUILD_ROOT/macos/lib/benchmark_main"
printf '#!/bin/bash\necho android\n' > "$BUILD_ROOT/android-arm64-v8a/platform/android/pso_benchmark"
printf 'plist' > "$BUILD_ROOT/ios-device/platform/ios/Release-iphoneos/PSOBenchmarkApp.app/Info.plist"
printf 'ipa' > "$BUILD_ROOT/ios-device/PSOBenchmarkApp.ipa"
printf 'exe' > "$BUILD_ROOT/windows/platform/windows/compression_benchmark_cli.exe"
chmod +x "$BUILD_ROOT/macos/lib/benchmark_main" "$BUILD_ROOT/android-arm64-v8a/platform/android/pso_benchmark"

BUILD_ROOT="$BUILD_ROOT" RELEASE_ROOT="$RELEASE_ROOT" \
  "$APP_ROOT/platform/release/assemble_release.sh"

for path in \
  "$RELEASE_ROOT/packages/macos/benchmark_main" \
  "$RELEASE_ROOT/packages/android/pso_benchmark" \
  "$RELEASE_ROOT/packages/ios/PSOBenchmarkApp.ipa" \
  "$RELEASE_ROOT/packages/ios/PSOBenchmarkApp.app/Info.plist" \
  "$RELEASE_ROOT/packages/windows/compression_benchmark_cli.exe" \
  "$RELEASE_ROOT/scripts/run_macos.sh" \
  "$RELEASE_ROOT/scripts/run_android.sh" \
  "$RELEASE_ROOT/scripts/run_android.ps1" \
  "$RELEASE_ROOT/scripts/run_ios.sh" \
  "$RELEASE_ROOT/scripts/run_ios.ps1" \
  "$RELEASE_ROOT/scripts/run_windows.ps1" \
  "$RELEASE_ROOT/manifest.txt"; do
  if [[ ! -e "$path" ]]; then
    echo "Expected release asset missing: $path"
    exit 1
  fi
done

if ! rg -q '^macos: packages/macos/benchmark_main$' "$RELEASE_ROOT/manifest.txt"; then
  echo "Expected macOS package entry in manifest"
  exit 1
fi

if ! rg -q '^ios_ipa: packages/ios/PSOBenchmarkApp.ipa$' "$RELEASE_ROOT/manifest.txt"; then
  echo "Expected iOS IPA entry in manifest"
  exit 1
fi
