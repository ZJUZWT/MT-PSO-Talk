#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
SOURCE_PATH="$APP_ROOT/lib/driver/common/device_info.cpp"

if rg -q 'info\\.device_model = "iOS Device";' "$SOURCE_PATH"; then
  echo "Expected iOS device metadata to stop hardcoding device_model=iOS Device"
  exit 1
fi

if rg -q 'info\\.soc = "Apple Silicon";' "$SOURCE_PATH"; then
  echo "Expected iOS device metadata to stop hardcoding soc=Apple Silicon"
  exit 1
fi

if ! rg -q 'hw\\.machine|uname' "$SOURCE_PATH"; then
  echo "Expected iOS device metadata to probe a real Apple device identifier"
  exit 1
fi
