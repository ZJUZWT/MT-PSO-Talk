#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
EDGE_BIN="/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge"
VIEWPORT_WIDTH=""
VIEWPORT_HEIGHT=""
DEFAULT_VIEWPORT_WIDTH="1280"
DEFAULT_VIEWPORT_HEIGHT="720"
VIRTUAL_TIME_BUDGET_MS=3500
PREFIX="stage-capture"
SOURCE_URL="http://127.0.0.1:4173/?step=page_09"
OUTDIR=""

usage() {
  cat <<'EOF'
Usage: capture_stage_png.sh [--url URL] [--outdir PATH] [--prefix NAME] [--width PX] [--height PX]

Captures a SlideApp stage-only PNG artifact using Microsoft Edge headless mode.

Defaults:
  --url     http://127.0.0.1:4173/?step=page_09
  --outdir  <repo>/ignore/slide-stage-captures/<timestamp>/
  --prefix  stage-capture
  --width   current front Edge window width (fallback 1280)
  --height  current front Edge window height (fallback 720)
EOF
}

ensure_source_url_reachable() {
  if ! curl --silent --show-error --fail --max-time 4 "$SOURCE_URL" >/dev/null; then
    echo "Source URL is not reachable: $SOURCE_URL" >&2
    echo "Hint: make sure SlideApp dev server is running (npm --prefix SlideApp run dev)." >&2
    exit 1
  fi
}

resolve_front_window_viewport() {
  local bounds_raw left top right bottom

  if bounds_raw="$(osascript -e 'tell application "Microsoft Edge" to get bounds of front window' 2>/dev/null)"; then
    IFS=', ' read -r left top right bottom <<< "$bounds_raw"
    if [[ -n "${left:-}" && -n "${top:-}" && -n "${right:-}" && -n "${bottom:-}" ]]; then
      printf '%s %s\n' "$((right - left))" "$((bottom - top))"
      return 0
    fi
  fi

  printf '%s %s\n' "$DEFAULT_VIEWPORT_WIDTH" "$DEFAULT_VIEWPORT_HEIGHT"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --url)
      SOURCE_URL="${2:-}"
      shift 2
      ;;
    --outdir)
      OUTDIR="${2:-}"
      shift 2
      ;;
    --prefix)
      PREFIX="${2:-}"
      shift 2
      ;;
    --width)
      VIEWPORT_WIDTH="${2:-}"
      shift 2
      ;;
    --height)
      VIEWPORT_HEIGHT="${2:-}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ ! -x "$EDGE_BIN" ]]; then
  echo "Missing Microsoft Edge binary at: $EDGE_BIN" >&2
  exit 1
fi

if [[ -z "$OUTDIR" ]]; then
  timestamp="$(date '+%Y%m%d-%H%M%S')"
  OUTDIR="$ROOT_DIR/ignore/slide-stage-captures/$timestamp"
fi

mkdir -p "$OUTDIR"
ensure_source_url_reachable

if [[ -z "$VIEWPORT_WIDTH" || -z "$VIEWPORT_HEIGHT" ]]; then
  read -r AUTO_WIDTH AUTO_HEIGHT < <(resolve_front_window_viewport)
  [[ -z "$VIEWPORT_WIDTH" ]] && VIEWPORT_WIDTH="$AUTO_WIDTH"
  [[ -z "$VIEWPORT_HEIGHT" ]] && VIEWPORT_HEIGHT="$AUTO_HEIGHT"
fi

CAPTURE_URL="$(python3 - "$SOURCE_URL" "$VIEWPORT_WIDTH" "$VIEWPORT_HEIGHT" <<'PY'
import sys
from urllib.parse import parse_qsl, urlencode, urlparse, urlunparse

source_url = sys.argv[1]
parsed = urlparse(source_url)
params = dict(parse_qsl(parsed.query, keep_blank_values=True))
params["surface"] = "stage"
params["captureWidth"] = sys.argv[2]
params["captureHeight"] = sys.argv[3]
query = urlencode(params)
print(urlunparse(parsed._replace(query=query)))
PY
)"

IMAGE_PATH="$OUTDIR/$PREFIX.png"
META_PATH="$OUTDIR/$PREFIX.txt"

"$EDGE_BIN" \
  --headless=new \
  --disable-gpu \
  --hide-scrollbars \
  --window-size="${VIEWPORT_WIDTH},${VIEWPORT_HEIGHT}" \
  --virtual-time-budget="${VIRTUAL_TIME_BUDGET_MS}" \
  --screenshot="$IMAGE_PATH" \
  "$CAPTURE_URL" >/dev/null

if [[ ! -f "$IMAGE_PATH" ]]; then
  echo "Failed to create screenshot at: $IMAGE_PATH" >&2
  exit 1
fi

cat >"$META_PATH" <<EOF
mode=edge-headless-stage
source_url=$SOURCE_URL
capture_url=$CAPTURE_URL
viewport_css=${VIEWPORT_WIDTH},${VIEWPORT_HEIGHT}
browser_chrome_included=no
image_path=$IMAGE_PATH
EOF

printf '%s\n' "$IMAGE_PATH"
