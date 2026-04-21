#!/bin/bash

benchmark_ios_lookup_device_field() {
  local json_path="$1"
  local target_identifier="$2"
  local field_path="$3"
  local index=0
  local identifier=""

  while identifier=$(/usr/bin/plutil -extract "result.devices.$index.identifier" raw -o - "$json_path" 2>/dev/null); do
    if [[ "$identifier" == "$target_identifier" ]]; then
      /usr/bin/plutil -extract "result.devices.$index.$field_path" raw -o - "$json_path"
      return 0
    fi
    index=$((index + 1))
  done

  return 1
}

benchmark_ios_select_available_device() {
  local json_path="$1"
  local index=0
  local identifier=""
  local tunnel_state=""

  while identifier=$(/usr/bin/plutil -extract "result.devices.$index.identifier" raw -o - "$json_path" 2>/dev/null); do
    tunnel_state=$(/usr/bin/plutil -extract "result.devices.$index.connectionProperties.tunnelState" raw -o - "$json_path" 2>/dev/null || true)
    if [[ "$tunnel_state" != "unavailable" ]]; then
      echo "$identifier"
      return 0
    fi
    index=$((index + 1))
  done

  return 1
}

benchmark_ios_lookup_device_name() {
  local json_path="$1"
  local target_identifier="$2"
  benchmark_ios_lookup_device_field "$json_path" "$target_identifier" "deviceProperties.name"
}

benchmark_ios_lookup_device_udid() {
  local json_path="$1"
  local target_identifier="$2"
  benchmark_ios_lookup_device_field "$json_path" "$target_identifier" "hardwareProperties.udid"
}

benchmark_ios_lookup_marketing_name() {
  local json_path="$1"
  local target_identifier="$2"
  benchmark_ios_lookup_device_field "$json_path" "$target_identifier" "hardwareProperties.marketingName"
}

benchmark_ios_lookup_product_type() {
  local json_path="$1"
  local target_identifier="$2"
  benchmark_ios_lookup_device_field "$json_path" "$target_identifier" "hardwareProperties.productType"
}

benchmark_ios_lookup_hardware_model() {
  local json_path="$1"
  local target_identifier="$2"
  benchmark_ios_lookup_device_field "$json_path" "$target_identifier" "hardwareProperties.hardwareModel"
}

benchmark_ios_bundle_identifier() {
  local app_path="$1"
  /usr/bin/plutil -extract CFBundleIdentifier raw -o - "$app_path/Info.plist"
}
