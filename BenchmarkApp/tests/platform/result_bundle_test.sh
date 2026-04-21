#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/result_bundle.sh"

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

export BENCHMARK_FIXED_TIMESTAMP="20260417T120500Z"
result_dir=$(benchmark_create_result_dir "$TMP_DIR/benchmark_results" "ios")
expected_dir="$TMP_DIR/benchmark_results/ios/20260417T120500Z"

if [[ "$result_dir" != "$expected_dir" ]]; then
  echo "Expected result dir $expected_dir, got: $result_dir"
  exit 1
fi

if [[ ! -d "$result_dir" ]]; then
  echo "Expected result dir to be created"
  exit 1
fi

benchmark_write_run_info \
  "$result_dir/run_info.txt" \
  "ios" \
  "Swann iPhone" \
  "00008110-TESTDEVICE" \
  "2026-04-17T12:05:00Z" \
  "2026-04-17T12:06:30Z" \
  "/tmp/PSOBenchmarkApp.app" \
  "benchmark_report.json" \
  "compression_results.csv"

if ! rg -q '^Platform: ios$' "$result_dir/run_info.txt"; then
  echo "Expected Platform line in run_info.txt"
  exit 1
fi

if ! rg -q '^Device: Swann iPhone$' "$result_dir/run_info.txt"; then
  echo "Expected Device line in run_info.txt"
  exit 1
fi

if ! rg -q '^Started At \(UTC\): 2026-04-17T12:05:00Z$' "$result_dir/run_info.txt"; then
  echo "Expected Started At line in run_info.txt"
  exit 1
fi

if ! rg -q '^Finished At \(UTC\): 2026-04-17T12:06:30Z$' "$result_dir/run_info.txt"; then
  echo "Expected Finished At line in run_info.txt"
  exit 1
fi

