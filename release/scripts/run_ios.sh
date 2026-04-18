#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
source "$SCRIPT_DIR/common/result_bundle.sh"
source "$SCRIPT_DIR/ios/ios_device_utils.sh"

APP_PATH="${1:-$SCRIPT_DIR/../packages/ios/PSOBenchmarkApp.app}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$SCRIPT_DIR/../results}"
POLL_INTERVAL_SECONDS="${POLL_INTERVAL_SECONDS:-5}"
POLL_TIMEOUT_SECONDS="${POLL_TIMEOUT_SECONDS:-900}"
DEVICE_OVERRIDE="${IOS_DEVICE:-}"

if [[ ! -d "$APP_PATH" ]]; then
  echo "iOS app bundle not found: $APP_PATH"
  exit 1
fi

BUNDLE_ID=$(benchmark_ios_bundle_identifier "$APP_PATH")
DEVICE_JSON=$(mktemp)
trap 'rm -f "$DEVICE_JSON"' EXIT
xcrun devicectl list devices --json-output "$DEVICE_JSON" >/dev/null

if [[ -n "$DEVICE_OVERRIDE" ]]; then
  DEVICE_IDENTIFIER="$DEVICE_OVERRIDE"
else
  if ! DEVICE_IDENTIFIER=$(benchmark_ios_select_available_device "$DEVICE_JSON"); then
    echo "No available iOS device found. Connect and unlock a device, then try again."
    xcrun devicectl list devices
    exit 1
  fi
fi

DEVICE_NAME=$(benchmark_ios_lookup_device_name "$DEVICE_JSON" "$DEVICE_IDENTIFIER" 2>/dev/null || echo "$DEVICE_IDENTIFIER")
DEVICE_UDID=$(benchmark_ios_lookup_device_udid "$DEVICE_JSON" "$DEVICE_IDENTIFIER" 2>/dev/null || echo "")
STARTED_AT=$(benchmark_now_iso_utc)
RESULT_DIR=$(benchmark_create_result_dir "$OUTPUT_ROOT" "ios")
LOG_PATH="$RESULT_DIR/console.log"
JSON_PATH="$RESULT_DIR/benchmark_report.json"
CSV_PATH="$RESULT_DIR/compression_results.csv"

echo "Preparing device $DEVICE_NAME..." | tee "$LOG_PATH"
xcrun devicectl device uninstall app --device "$DEVICE_IDENTIFIER" "$BUNDLE_ID" >/dev/null 2>&1 || true
xcrun devicectl device install app --device "$DEVICE_IDENTIFIER" "$APP_PATH" | tee -a "$LOG_PATH"
xcrun devicectl device process launch --device "$DEVICE_IDENTIFIER" --terminate-existing "$BUNDLE_ID" | tee -a "$LOG_PATH"

deadline=$((SECONDS + POLL_TIMEOUT_SECONDS))
while (( SECONDS < deadline )); do
  json_ready=0
  csv_ready=0

  if xcrun devicectl device copy from \
    --device "$DEVICE_IDENTIFIER" \
    --domain-type appDataContainer \
    --domain-identifier "$BUNDLE_ID" \
    --source "Documents/compression_results.json" \
    --destination "$JSON_PATH" >/dev/null 2>&1; then
    json_ready=1
  fi

  if xcrun devicectl device copy from \
    --device "$DEVICE_IDENTIFIER" \
    --domain-type appDataContainer \
    --domain-identifier "$BUNDLE_ID" \
    --source "Documents/compression_results.csv" \
    --destination "$CSV_PATH" >/dev/null 2>&1; then
    csv_ready=1
  fi

  if [[ "$json_ready" -eq 1 && "$csv_ready" -eq 1 ]]; then
    break
  fi

  sleep "$POLL_INTERVAL_SECONDS"
done

if [[ ! -f "$JSON_PATH" || ! -f "$CSV_PATH" ]]; then
  echo "Timed out waiting for iOS benchmark results."
  echo "Check the app on device and inspect: $RESULT_DIR"
  exit 1
fi

FINISHED_AT=$(benchmark_now_iso_utc)
benchmark_write_run_info \
  "$RESULT_DIR/run_info.txt" \
  "ios" \
  "$DEVICE_NAME" \
  "$DEVICE_UDID" \
  "$STARTED_AT" \
  "$FINISHED_AT" \
  "$APP_PATH" \
  "benchmark_report.json" \
  "compression_results.csv"
echo "Bundle Identifier: $BUNDLE_ID" >> "$RESULT_DIR/run_info.txt"
echo "Device Identifier: $DEVICE_IDENTIFIER" >> "$RESULT_DIR/run_info.txt"

benchmark_print_run_summary "ios" "$DEVICE_NAME" "$STARTED_AT" "$FINISHED_AT" "$RESULT_DIR"
