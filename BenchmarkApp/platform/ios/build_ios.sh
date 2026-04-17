#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
BUILD_ROOT="${BUILD_ROOT:-$APP_ROOT/../build/BenchmarkApp}"
MODE="${1:-device}"
CONFIGURATION="${CONFIGURATION:-Release}"
TARGET_NAME="PSOBenchmarkApp"

case "$MODE" in
  device)
    SDK="iphoneos"
    BUILD_DIR="$BUILD_ROOT/ios-device"
    ;;
  simulator|sim)
    SDK="iphonesimulator"
    BUILD_DIR="$BUILD_ROOT/ios-sim"
    ;;
  *)
    echo "Unsupported mode: $MODE"
    echo "Usage: $0 [device|simulator]"
    exit 1
    ;;
esac

IOS_BUNDLE_IDENTIFIER="${IOS_BUNDLE_IDENTIFIER:-com.pso.benchmark}"
IOS_CODE_SIGN_STYLE="${IOS_CODE_SIGN_STYLE:-Automatic}"
IOS_DEVELOPMENT_TEAM="${IOS_DEVELOPMENT_TEAM:-}"
IOS_CODE_SIGN_IDENTITY="${IOS_CODE_SIGN_IDENTITY:-}"
IOS_PROVISIONING_PROFILE_SPECIFIER="${IOS_PROVISIONING_PROFILE_SPECIFIER:-}"

if [[ "$MODE" == "device" && -z "$IOS_DEVELOPMENT_TEAM" ]]; then
  echo "IOS_DEVELOPMENT_TEAM is required for signed device builds."
  echo "Example: IOS_DEVELOPMENT_TEAM=ABCDE12345 $0 device"
  exit 1
fi

cmake -S "$APP_ROOT" -B "$BUILD_DIR" -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_SYSROOT="$SDK" \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -DIOS_BUNDLE_IDENTIFIER="$IOS_BUNDLE_IDENTIFIER" \
  -DIOS_DEVELOPMENT_TEAM="$IOS_DEVELOPMENT_TEAM" \
  -DIOS_CODE_SIGN_IDENTITY="$IOS_CODE_SIGN_IDENTITY" \
  -DIOS_PROVISIONING_PROFILE_SPECIFIER="$IOS_PROVISIONING_PROFILE_SPECIFIER" \
  -DIOS_CODE_SIGN_STYLE="$IOS_CODE_SIGN_STYLE"

XCODE_ARGS=(
  -project "$BUILD_DIR/PSOBenchmark.xcodeproj"
  -target "$TARGET_NAME"
  -configuration "$CONFIGURATION"
  -sdk "$SDK"
)

if [[ "$MODE" == "device" ]]; then
  XCODE_ARGS+=(
    "DEVELOPMENT_TEAM=$IOS_DEVELOPMENT_TEAM"
    "PRODUCT_BUNDLE_IDENTIFIER=$IOS_BUNDLE_IDENTIFIER"
    "CODE_SIGN_STYLE=$IOS_CODE_SIGN_STYLE"
    -allowProvisioningUpdates
    build
  )

  if [[ -n "$IOS_CODE_SIGN_IDENTITY" ]]; then
    XCODE_ARGS+=("CODE_SIGN_IDENTITY=$IOS_CODE_SIGN_IDENTITY")
  fi

  if [[ -n "$IOS_PROVISIONING_PROFILE_SPECIFIER" ]]; then
    XCODE_ARGS+=("PROVISIONING_PROFILE_SPECIFIER=$IOS_PROVISIONING_PROFILE_SPECIFIER")
  fi
else
  XCODE_ARGS+=(
    CODE_SIGNING_ALLOWED=NO
    CODE_SIGNING_REQUIRED=NO
    build
  )
fi

xcodebuild "${XCODE_ARGS[@]}"

APP_PATH="$BUILD_DIR/platform/ios/${CONFIGURATION}-${SDK}/${TARGET_NAME}.app"
echo "iOS app built at: $APP_PATH"
