#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/result_bundle.sh"

BINARY=${1:-"$APP_ROOT/../build/BenchmarkApp/android-arm64-v8a/platform/android/pso_benchmark"}
DEVICE_DIR="${ANDROID_DEVICE_DIR:-/data/local/tmp/pso_benchmark}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$APP_ROOT/../release/results}"

if [[ ! -f "$BINARY" ]]; then
  echo "Android benchmark binary not found: $BINARY"
  exit 1
fi

ANDROID_SERIAL_VALUE="${ANDROID_SERIAL:-$(adb devices | awk 'NR > 1 && $2 == "device" { print $1; exit }')}"
if [[ -z "$ANDROID_SERIAL_VALUE" ]]; then
  echo "No Android device detected. Connect a device or set ANDROID_SERIAL."
  exit 1
fi

ADB=(adb -s "$ANDROID_SERIAL_VALUE")
DEVICE_NAME=$("${ADB[@]}" shell getprop ro.product.model | tr -d '\r')
ANDROID_VERSION=$("${ADB[@]}" shell getprop ro.build.version.release | tr -d '\r')
STARTED_AT=$(benchmark_now_iso_utc)
RESULT_DIR=$(benchmark_create_result_dir "$OUTPUT_ROOT" "android")
LOG_PATH="$RESULT_DIR/console.log"

# Create device directory
"${ADB[@]}" shell "mkdir -p $DEVICE_DIR"

echo "Pushing binary..."
"${ADB[@]}" push "$BINARY" "$DEVICE_DIR/pso_benchmark" | tee "$LOG_PATH"
"${ADB[@]}" shell "chmod +x $DEVICE_DIR/pso_benchmark"

echo "Running benchmark..."
"${ADB[@]}" shell "cd $DEVICE_DIR && ./pso_benchmark --json results.json --csv results.csv" 2>&1 | tee -a "$LOG_PATH"

# Pull results
echo "Pulling results..."
"${ADB[@]}" pull "$DEVICE_DIR/results.json" "$RESULT_DIR/benchmark_report.json" | tee -a "$LOG_PATH"
"${ADB[@]}" pull "$DEVICE_DIR/results.csv" "$RESULT_DIR/compression_results.csv" | tee -a "$LOG_PATH"

FINISHED_AT=$(benchmark_now_iso_utc)
benchmark_write_run_info \
  "$RESULT_DIR/run_info.txt" \
  "android" \
  "$DEVICE_NAME" \
  "$ANDROID_SERIAL_VALUE" \
  "$STARTED_AT" \
  "$FINISHED_AT" \
  "$BINARY" \
  "benchmark_report.json" \
  "compression_results.csv"
echo "OS Version: Android $ANDROID_VERSION" >> "$RESULT_DIR/run_info.txt"

benchmark_print_run_summary "android" "$DEVICE_NAME" "$STARTED_AT" "$FINISHED_AT" "$RESULT_DIR"
