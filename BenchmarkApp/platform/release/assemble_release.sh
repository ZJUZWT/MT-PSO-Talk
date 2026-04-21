#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/common/build_settings.sh"

BUILD_ROOT=$(benchmark_build_root "$APP_ROOT")
RELEASE_ROOT="${RELEASE_ROOT:-$APP_ROOT/../release}"
APPLE_CONFIGURATION=$(benchmark_apple_configuration)
WINDOWS_CONFIGURATION=$(benchmark_windows_configuration)

PACKAGES_DIR="$RELEASE_ROOT/packages"
SCRIPTS_DIR="$RELEASE_ROOT/scripts"
MANIFEST_PATH="$RELEASE_ROOT/manifest.txt"

copy_if_present() {
  local source_path="$1"
  local destination_path="$2"

  if [[ -e "$source_path" ]]; then
    mkdir -p "$(dirname "$destination_path")"
    cp -R "$source_path" "$destination_path"
    return 0
  fi

  return 1
}

write_manifest_line() {
  local key="$1"
  local value="$2"
  echo "$key: $value" >> "$MANIFEST_PATH"
}

rm -rf "$RELEASE_ROOT"
mkdir -p "$PACKAGES_DIR/macos" "$PACKAGES_DIR/android" "$PACKAGES_DIR/ios" "$PACKAGES_DIR/windows"
mkdir -p "$SCRIPTS_DIR/common" "$SCRIPTS_DIR/ios"

cp "$APP_ROOT/platform/common/result_bundle.sh" "$SCRIPTS_DIR/common/result_bundle.sh"
cp "$APP_ROOT/platform/ios/ios_device_utils.sh" "$SCRIPTS_DIR/ios/ios_device_utils.sh"
cp "$APP_ROOT/platform/android/run_benchmark.ps1" "$SCRIPTS_DIR/run_android.ps1"
cp "$APP_ROOT/platform/ios/run_benchmark.ps1" "$SCRIPTS_DIR/run_ios.ps1"
chmod +x "$SCRIPTS_DIR/common/result_bundle.sh" "$SCRIPTS_DIR/ios/ios_device_utils.sh"

{
  echo "Release Root: $RELEASE_ROOT"
  echo "Build Root: $BUILD_ROOT"
  echo "Build Type: $(benchmark_build_type)"
  echo "Apple Configuration: $APPLE_CONFIGURATION"
  echo "Windows Configuration: $WINDOWS_CONFIGURATION"
} > "$MANIFEST_PATH"

if copy_if_present "$BUILD_ROOT/macos/lib/benchmark_main" "$PACKAGES_DIR/macos/benchmark_main"; then
  chmod +x "$PACKAGES_DIR/macos/benchmark_main"
  write_manifest_line "macos" "packages/macos/benchmark_main"
else
  write_manifest_line "macos" "missing"
fi

if copy_if_present "$BUILD_ROOT/android-arm64-v8a/platform/android/pso_benchmark" "$PACKAGES_DIR/android/pso_benchmark"; then
  chmod +x "$PACKAGES_DIR/android/pso_benchmark"
  write_manifest_line "android" "packages/android/pso_benchmark"
else
  write_manifest_line "android" "missing"
fi

if copy_if_present "$(benchmark_ios_app_path "$APP_ROOT" device)" "$PACKAGES_DIR/ios/PSOBenchmarkApp.app"; then
  write_manifest_line "ios_app" "packages/ios/PSOBenchmarkApp.app"
else
  write_manifest_line "ios_app" "missing"
fi

if copy_if_present "$BUILD_ROOT/ios-device/PSOBenchmarkApp.ipa" "$PACKAGES_DIR/ios/PSOBenchmarkApp.ipa"; then
  write_manifest_line "ios_ipa" "packages/ios/PSOBenchmarkApp.ipa"
else
  write_manifest_line "ios_ipa" "missing"
fi

windows_binary_candidates=()
while IFS= read -r candidate; do
  windows_binary_candidates+=("$candidate")
done < <(benchmark_windows_binary_candidates "$BUILD_ROOT" "$WINDOWS_CONFIGURATION")
if windows_binary_path=$(benchmark_first_existing_path "${windows_binary_candidates[@]}"); then
  copy_if_present "$windows_binary_path" "$PACKAGES_DIR/windows/compression_benchmark_cli.exe" >/dev/null
  write_manifest_line "windows" "packages/windows/compression_benchmark_cli.exe"
else
  write_manifest_line "windows" "missing"
fi

cat > "$SCRIPTS_DIR/run_macos.sh" <<'EOF'
#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
source "$SCRIPT_DIR/common/result_bundle.sh"

BINARY_PATH="${1:-$SCRIPT_DIR/../packages/macos/benchmark_main}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$SCRIPT_DIR/../results}"

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
EOF

cat > "$SCRIPTS_DIR/run_android.sh" <<'EOF'
#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
source "$SCRIPT_DIR/common/result_bundle.sh"

BINARY_PATH="${1:-$SCRIPT_DIR/../packages/android/pso_benchmark}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$SCRIPT_DIR/../results}"
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
EOF

cat > "$SCRIPTS_DIR/run_ios.sh" <<'EOF'
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
EOF

cat > "$SCRIPTS_DIR/run_windows.ps1" <<'EOF'
param(
    [string]$BinaryPath = "",
    [string]$OutputRoot = ""
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not $BinaryPath) {
    $BinaryPath = Join-Path $scriptDir "..\packages\windows\compression_benchmark_cli.exe"
}

if (-not (Test-Path $BinaryPath)) {
    throw "Windows benchmark binary not found: $BinaryPath"
}

if (-not $OutputRoot) {
    $OutputRoot = Join-Path $scriptDir "..\results"
}

$startedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$dirTimestamp = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")
$resultDir = Join-Path $OutputRoot "windows\$dirTimestamp"
$null = New-Item -ItemType Directory -Path $resultDir -Force

$jsonPath = Join-Path $resultDir "benchmark_report.json"
$csvPath = Join-Path $resultDir "compression_results.csv"
$logPath = Join-Path $resultDir "console.log"

& $BinaryPath --json $jsonPath --csv $csvPath | Tee-Object -FilePath $logPath

$finishedAt = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")
$runInfoPath = Join-Path $resultDir "run_info.txt"
@(
    "Platform: windows"
    "Device: $env:COMPUTERNAME"
    "Device ID: $env:COMPUTERNAME"
    "Started At (UTC): $startedAt"
    "Finished At (UTC): $finishedAt"
    "Source Path: $BinaryPath"
    "JSON: benchmark_report.json"
    "CSV: compression_results.csv"
) | Set-Content -Path $runInfoPath

Write-Host "=== Benchmark Complete ==="
Write-Host "Platform: windows"
Write-Host "Device: $env:COMPUTERNAME"
Write-Host "Started At (UTC): $startedAt"
Write-Host "Finished At (UTC): $finishedAt"
Write-Host "Results Directory: $resultDir"
EOF

chmod +x \
  "$SCRIPTS_DIR/run_macos.sh" \
  "$SCRIPTS_DIR/run_android.sh" \
  "$SCRIPTS_DIR/run_ios.sh"

echo "Release assembled at: $RELEASE_ROOT"
