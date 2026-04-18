#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
source "$SCRIPT_DIR/common/result_bundle.sh"

BINARY_PATH="${1:-$SCRIPT_DIR/../packages/android/pso_benchmark}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$SCRIPT_DIR/../../benchmark_results}"
DEVICE_DIR="${ANDROID_DEVICE_DIR:-/data/local/tmp/pso_benchmark}"

if [[ ! -f "$BINARY_PATH" ]]; then
  echo "Android benchmark binary not found: $BINARY_PATH"
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

"${ADB[@]}" shell "mkdir -p $DEVICE_DIR"
"${ADB[@]}" push "$BINARY_PATH" "$DEVICE_DIR/pso_benchmark" | tee "$LOG_PATH"
"${ADB[@]}" shell "chmod +x $DEVICE_DIR/pso_benchmark"
"${ADB[@]}" shell "cd $DEVICE_DIR && ./pso_benchmark --json results.json --csv results.csv" 2>&1 | tee -a "$LOG_PATH"
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
  "$BINARY_PATH" \
  "benchmark_report.json" \
  "compression_results.csv"
echo "OS Version: Android $ANDROID_VERSION" >> "$RESULT_DIR/run_info.txt"

benchmark_print_run_summary "android" "$DEVICE_NAME" "$STARTED_AT" "$FINISHED_AT" "$RESULT_DIR"
