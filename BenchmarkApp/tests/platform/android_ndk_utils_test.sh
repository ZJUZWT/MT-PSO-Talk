#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
source "$APP_ROOT/platform/android/android_ndk_utils.sh"

TMP_DIR=$(mktemp -d)
trap 'rm -rf "$TMP_DIR"' EXIT

cat > "$TMP_DIR/sdkmanager-list.txt" <<'EOF'
  ndk;28.2.13676358                                                               | 28.2.13676358     | NDK (Side by side) 28.2.13676358
  ndk;29.0.13113456                                                               | 29.0.13113456 rc1 | NDK (Side by side) 29.0.13113456
  ndk;29.0.14206865                                                               | 29.0.14206865     | NDK (Side by side) 29.0.14206865
  ndk;30.0.14904198                                                               | 30.0.14904198 rc1 | NDK (Side by side) 30.0.14904198
EOF

selected_package=$(benchmark_select_latest_stable_ndk_package < "$TMP_DIR/sdkmanager-list.txt")
if [[ "$selected_package" != "ndk;29.0.14206865" ]]; then
  echo "Expected stable NDK package ndk;29.0.14206865, got: $selected_package"
  exit 1
fi

mkdir -p "$TMP_DIR/sdk/ndk/30.0.14904198" "$TMP_DIR/sdk/ndk/29.0.14206865"
cat > "$TMP_DIR/sdk/ndk/30.0.14904198/source.properties" <<'EOF'
Pkg.Desc = Android NDK
Pkg.Revision = 30.0.14904198-beta1
Pkg.BaseRevision = 30.0.14904198
Pkg.ReleaseName = r30-beta1
EOF
cat > "$TMP_DIR/sdk/ndk/29.0.14206865/source.properties" <<'EOF'
Pkg.Desc = Android NDK
Pkg.Revision = 29.0.14206865
Pkg.BaseRevision = 29.0.14206865
Pkg.ReleaseName = r29
EOF

selected_root=$(benchmark_find_preferred_ndk_root "$TMP_DIR/sdk")
expected_root="$TMP_DIR/sdk/ndk/29.0.14206865"
if [[ "$selected_root" != "$expected_root" ]]; then
  echo "Expected stable NDK root $expected_root, got: $selected_root"
  exit 1
fi
