#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$SCRIPT_DIR/android_ndk_utils.sh"

SDK_ROOT="${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}"
CMDLINE_ZIP="${CMDLINE_ZIP:-$HOME/Downloads/commandlinetools-mac-14742923_latest.zip}"
CMDLINE_URL="${CMDLINE_URL:-https://dl.google.com/android/repository/commandlinetools-mac-14742923_latest.zip}"
CMDLINE_DIR="$SDK_ROOT/cmdline-tools"
JAVA_HOME_DEFAULT="/opt/homebrew/opt/openjdk@17/libexec/openjdk.jdk/Contents/Home"

if [[ -z "${JAVA_HOME:-}" && -x "$JAVA_HOME_DEFAULT/bin/java" ]]; then
  export JAVA_HOME="$JAVA_HOME_DEFAULT"
  export PATH="$JAVA_HOME/bin:$PATH"
fi

if ! command -v java >/dev/null 2>&1; then
  echo "Java runtime not found."
  echo "Install openjdk@17 first, for example: brew install openjdk@17"
  exit 1
fi

mkdir -p "$CMDLINE_DIR"

if [[ ! -f "$CMDLINE_ZIP" ]]; then
  curl -L "$CMDLINE_URL" -o "$CMDLINE_ZIP"
fi

rm -rf "$CMDLINE_DIR/latest.tmp"
mkdir -p "$CMDLINE_DIR/latest.tmp"
unzip -q -o "$CMDLINE_ZIP" -d "$CMDLINE_DIR/latest.tmp"
rm -rf "$CMDLINE_DIR/latest"
mv "$CMDLINE_DIR/latest.tmp/cmdline-tools" "$CMDLINE_DIR/latest"
rmdir "$CMDLINE_DIR/latest.tmp" 2>/dev/null || true

SDKMANAGER="$CMDLINE_DIR/latest/bin/sdkmanager"
LATEST_NDK="${ANDROID_NDK_PACKAGE:-$("$SDKMANAGER" --list | benchmark_select_latest_stable_ndk_package)}"

if [[ -z "$LATEST_NDK" ]]; then
  echo "Unable to determine an Android NDK package from sdkmanager --list."
  exit 1
fi

yes | "$SDKMANAGER" --licenses >/dev/null || true
"$SDKMANAGER" \
  "platform-tools" \
  "platforms;android-24" \
  "$LATEST_NDK"

echo "Android SDK root: $SDK_ROOT"
echo "Installed NDK package: $LATEST_NDK"
echo "Installed NDK path: $SDK_ROOT/ndk/${LATEST_NDK#ndk;}"
