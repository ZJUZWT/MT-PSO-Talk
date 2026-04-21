#!/bin/bash

benchmark_dir_timestamp() {
  if [[ -n "${BENCHMARK_FIXED_TIMESTAMP:-}" ]]; then
    echo "$BENCHMARK_FIXED_TIMESTAMP"
    return 0
  fi

  date -u +"%Y%m%dT%H%M%SZ"
}

benchmark_now_iso_utc() {
  date -u +"%Y-%m-%dT%H:%M:%SZ"
}

benchmark_create_result_dir() {
  local output_root="$1"
  local platform="$2"
  local timestamp
  local result_dir

  timestamp=$(benchmark_dir_timestamp)
  result_dir="$output_root/$platform/$timestamp"
  mkdir -p "$result_dir"
  echo "$result_dir"
}

benchmark_write_run_info() {
  local output_path="$1"
  local platform="$2"
  local device_name="$3"
  local device_id="$4"
  local started_at="$5"
  local finished_at="$6"
  local source_path="$7"
  local json_name="$8"
  local csv_name="$9"

  cat > "$output_path" <<EOF
Platform: $platform
Device: $device_name
Device ID: $device_id
Started At (UTC): $started_at
Finished At (UTC): $finished_at
Source Path: $source_path
JSON: $json_name
CSV: $csv_name
EOF
}

benchmark_print_run_summary() {
  local platform="$1"
  local device_name="$2"
  local started_at="$3"
  local finished_at="$4"
  local result_dir="$5"

  cat <<EOF
=== Benchmark Complete ===
Platform: $platform
Device: $device_name
Started At (UTC): $started_at
Finished At (UTC): $finished_at
Results Directory: $result_dir
EOF
}
