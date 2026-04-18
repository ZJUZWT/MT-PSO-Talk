#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/ios/ios_device_utils.sh"

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/devices.json" <<'EOF'
{
  "result" : {
    "devices" : [
      {
        "identifier" : "UNAVAILABLE-DEVICE",
        "connectionProperties" : {
          "tunnelState" : "unavailable"
        },
        "deviceProperties" : {
          "name" : "Unavailable iPad"
        },
        "hardwareProperties" : {
          "udid" : "UNAVAILABLE-UDID"
        }
      },
      {
        "identifier" : "AVAILABLE-DEVICE",
        "connectionProperties" : {
          "tunnelState" : "connected"
        },
        "deviceProperties" : {
          "name" : "Bench iPhone"
        },
        "hardwareProperties" : {
          "hardwareModel" : "D83AP",
          "marketingName" : "Bench iPhone 15 Pro",
          "productType" : "iPhone16,1",
          "udid" : "AVAILABLE-UDID"
        }
      }
    ]
  }
}
EOF

selected_device=$(benchmark_ios_select_available_device "$TMP_DIR/devices.json")
if [[ "$selected_device" != "AVAILABLE-DEVICE" ]]; then
  echo "Expected AVAILABLE-DEVICE, got: $selected_device"
  exit 1
fi

selected_name=$(benchmark_ios_lookup_device_name "$TMP_DIR/devices.json" "AVAILABLE-DEVICE")
if [[ "$selected_name" != "Bench iPhone" ]]; then
  echo "Expected Bench iPhone, got: $selected_name"
  exit 1
fi

selected_udid=$(benchmark_ios_lookup_device_udid "$TMP_DIR/devices.json" "AVAILABLE-DEVICE")
if [[ "$selected_udid" != "AVAILABLE-UDID" ]]; then
  echo "Expected AVAILABLE-UDID, got: $selected_udid"
  exit 1
fi

selected_marketing_name=$(benchmark_ios_lookup_marketing_name "$TMP_DIR/devices.json" "AVAILABLE-DEVICE")
if [[ "$selected_marketing_name" != "Bench iPhone 15 Pro" ]]; then
  echo "Expected Bench iPhone 15 Pro, got: $selected_marketing_name"
  exit 1
fi

selected_product_type=$(benchmark_ios_lookup_product_type "$TMP_DIR/devices.json" "AVAILABLE-DEVICE")
if [[ "$selected_product_type" != "iPhone16,1" ]]; then
  echo "Expected iPhone16,1, got: $selected_product_type"
  exit 1
fi

selected_hardware_model=$(benchmark_ios_lookup_hardware_model "$TMP_DIR/devices.json" "AVAILABLE-DEVICE")
if [[ "$selected_hardware_model" != "D83AP" ]]; then
  echo "Expected D83AP, got: $selected_hardware_model"
  exit 1
fi

cat > "$TMP_DIR/no_devices.json" <<'EOF'
{
  "result" : {
    "devices" : [
      {
        "identifier" : "UNAVAILABLE-DEVICE",
        "connectionProperties" : {
          "tunnelState" : "unavailable"
        },
        "deviceProperties" : {
          "name" : "Unavailable iPad"
        },
        "hardwareProperties" : {
          "udid" : "UNAVAILABLE-UDID"
        }
      }
    ]
  }
}
EOF

if benchmark_ios_select_available_device "$TMP_DIR/no_devices.json" >/dev/null 2>&1; then
  echo "Expected selection to fail when only unavailable devices exist"
  exit 1
fi

mkdir -p "$TMP_DIR/TestApp.app"
cat > "$TMP_DIR/TestApp.app/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>com.example.bench</string>
</dict>
</plist>
EOF

bundle_id=$(benchmark_ios_bundle_identifier "$TMP_DIR/TestApp.app")
if [[ "$bundle_id" != "com.example.bench" ]]; then
  echo "Expected bundle id com.example.bench, got: $bundle_id"
  exit 1
fi
