#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/result_bundle.sh"

DEFAULT_BINARY="$APP_ROOT/../build/BenchmarkApp/macos/lib/benchmark_main"
LEGACY_BINARY="$APP_ROOT/../build/benchmark/lib/benchmark_main"
OUTPUT_ROOT="${OUTPUT_ROOT:-$APP_ROOT/../benchmark_results}"

BINARY_PATH="${1:-}"
if [[ -z "$BINARY_PATH" ]]; then
  if [[ -x "$DEFAULT_BINARY" ]]; then
    BINARY_PATH="$DEFAULT_BINARY"
  else
    BINARY_PATH="$LEGACY_BINARY"
  fi
fi

if [[ ! -x "$BINARY_PATH" ]]; then
  echo "macOS benchmark binary not found: $BINARY_PATH"
  exit 1
fi

DEVICE_NAME=$(scutil --get ComputerName 2>/dev/null || hostname)
DEVICE_ID=$(sysctl -n hw.model 2>/dev/null || echo "mac-host")
OS_VERSION=$(sw_vers -productVersion 2>/dev/null || echo "unknown")
STARTED_AT=$(benchmark_now_iso_utc)
RESULT_DIR=$(benchmark_create_result_dir "$OUTPUT_ROOT" "macos")
LOG_PATH="$RESULT_DIR/console.log"
JSON_PATH="$RESULT_DIR/benchmark_report.json"
CSV_PATH="$RESULT_DIR/compression_results.csv"

"$BINARY_PATH" --json "$JSON_PATH" --csv "$CSV_PATH" | tee "$LOG_PATH"

FINISHED_AT=$(benchmark_now_iso_utc)
benchmark_write_run_info \
  "$RESULT_DIR/run_info.txt" \
  "macos" \
  "$DEVICE_NAME" \
  "$DEVICE_ID" \
  "$STARTED_AT" \
  "$FINISHED_AT" \
  "$BINARY_PATH" \
  "benchmark_report.json" \
  "compression_results.csv"
echo "OS Version: macOS $OS_VERSION" >> "$RESULT_DIR/run_info.txt"

benchmark_print_run_summary "macos" "$DEVICE_NAME" "$STARTED_AT" "$FINISHED_AT" "$RESULT_DIR"
