#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
EDGE_WINDOW_CAPTURE_SCRIPT="$HOME/.codex/skills/fetch-edge-image/scripts/capture_edge_window.sh"
BROWSER_CAPTURE_RECEIVER_SCRIPT="$ROOT_DIR/scripts/slide-geometry-harness/receive_capture_png.py"
EDGE_BIN="/Applications/Microsoft Edge.app/Contents/MacOS/Microsoft Edge"
MODE="front-window"
OUTDIR=""
PREFIX="review-capture"
SCOPE="page"
SOURCE_URL="http://127.0.0.1:4173/?step=page_09"
VIRTUAL_TIME_BUDGET_MS=8000
RECEIVER_TIMEOUT_SECONDS=35
EDGE_LOG_TAIL_LINES=40
DEFAULT_VIEWPORT_WIDTH="1280"
DEFAULT_VIEWPORT_HEIGHT="720"

usage() {
  cat <<'EOF'
Usage: capture_review_png.sh [--mode front-window|browser-api] [--scope page] [--url URL] [--outdir PATH] [--prefix NAME] [--virtual-time-budget-ms N] [--receiver-timeout-seconds N]

Capture a review-ready PNG for SlideApp harness work.

Modes:
  front-window   Capture the currently visible Microsoft Edge front window.
                 This is the primary mode because it matches what the user actually sees.
  browser-api    Ask SlideApp itself to export a PNG via html-to-image and post it
                 to a local receiver. This is the stable automation path for agents.
EOF
}

resolve_front_edge_viewport() {
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

ensure_source_url_reachable() {
  if ! curl --silent --show-error --fail --max-time 4 "$SOURCE_URL" >/dev/null; then
    echo "Source URL is not reachable: $SOURCE_URL" >&2
    echo "Hint: make sure SlideApp dev server is running (npm --prefix SlideApp run dev)." >&2
    exit 1
  fi
}

build_browser_capture_url() {
  python3 - "$SOURCE_URL" "$1" "$2" "$3" <<'PY'
import sys
from urllib.parse import parse_qsl, urlencode, urlparse, urlunparse

source_url, capture_post_url, capture_scope, viewport = sys.argv[1:5]
parsed = urlparse(source_url)
params = dict(parse_qsl(parsed.query, keep_blank_values=True))
params["capture"] = "1"
params["captureScope"] = capture_scope
params["captureTransport"] = "post"
params["capturePostUrl"] = capture_post_url
params["captureViewport"] = viewport
print(urlunparse(parsed._replace(query=urlencode(params))))
PY
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --mode)
      MODE="${2:-}"
      shift 2
      ;;
    --scope)
      SCOPE="${2:-}"
      shift 2
      ;;
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
    --virtual-time-budget-ms)
      VIRTUAL_TIME_BUDGET_MS="${2:-}"
      shift 2
      ;;
    --receiver-timeout-seconds)
      RECEIVER_TIMEOUT_SECONDS="${2:-}"
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

if [[ "$SCOPE" != "page" ]]; then
  echo "Stage capture has been disabled. Use --scope page." >&2
  exit 1
fi

case "$MODE" in
  front-window)
    if [[ ! -x "$EDGE_WINDOW_CAPTURE_SCRIPT" ]]; then
      echo "Missing Edge window capture helper: $EDGE_WINDOW_CAPTURE_SCRIPT" >&2
      exit 1
    fi

    if [[ -n "$OUTDIR" ]]; then
      exec "$EDGE_WINDOW_CAPTURE_SCRIPT" --mode screenshot --outdir "$OUTDIR" --prefix "$PREFIX"
    fi

    exec "$EDGE_WINDOW_CAPTURE_SCRIPT" --mode screenshot --prefix "$PREFIX"
    ;;
  browser-api)
    if [[ ! -f "$BROWSER_CAPTURE_RECEIVER_SCRIPT" ]]; then
      echo "Missing browser capture receiver: $BROWSER_CAPTURE_RECEIVER_SCRIPT" >&2
      exit 1
    fi

    if [[ ! -x "$EDGE_BIN" ]]; then
      echo "Missing Microsoft Edge binary at: $EDGE_BIN" >&2
      exit 1
    fi

    if [[ "$SCOPE" != "page" ]]; then
      echo "Unsupported browser-api scope: $SCOPE" >&2
      exit 1
    fi

    if [[ -z "$OUTDIR" ]]; then
      timestamp="$(date '+%Y%m%d-%H%M%S')"
      OUTDIR="$ROOT_DIR/ignore/browser-api-captures/$timestamp"
    fi

    mkdir -p "$OUTDIR"
    ensure_source_url_reachable

    read -r VIEWPORT_WIDTH VIEWPORT_HEIGHT < <(resolve_front_edge_viewport)

    TMP_DIR="$(mktemp -d)"
    PORT_FILE="$TMP_DIR/port.txt"
    RESULT_FILE="$TMP_DIR/result.txt"
    EDGE_LOG_FILE="$TMP_DIR/edge.log"

    cleanup() {
      if [[ -n "${RECEIVER_PID:-}" ]] && kill -0 "$RECEIVER_PID" 2>/dev/null; then
        kill "$RECEIVER_PID" 2>/dev/null || true
      fi
      rm -rf "$TMP_DIR"
    }
    trap cleanup EXIT

    python3 "$BROWSER_CAPTURE_RECEIVER_SCRIPT" \
      --outdir "$OUTDIR" \
      --prefix "$PREFIX" \
      --port-file "$PORT_FILE" \
      --result-file "$RESULT_FILE" \
      --timeout-seconds "$RECEIVER_TIMEOUT_SECONDS" &
    RECEIVER_PID=$!

    for _ in {1..100}; do
      if [[ -s "$PORT_FILE" ]]; then
        break
      fi
      sleep 0.05
    done

    if [[ ! -s "$PORT_FILE" ]]; then
      echo "Timed out waiting for local capture receiver port" >&2
      exit 1
    fi

    RECEIVER_PORT="$(<"$PORT_FILE")"
    CAPTURE_POST_URL="http://127.0.0.1:${RECEIVER_PORT}/capture"
    CAPTURE_URL="$(build_browser_capture_url "$CAPTURE_POST_URL" "$SCOPE" "${VIEWPORT_WIDTH}x${VIEWPORT_HEIGHT}")"

    "$EDGE_BIN" \
      --headless=new \
      --disable-gpu \
      --hide-scrollbars \
      --window-size="${VIEWPORT_WIDTH},${VIEWPORT_HEIGHT}" \
      --virtual-time-budget="${VIRTUAL_TIME_BUDGET_MS}" \
      --dump-dom \
      "$CAPTURE_URL" >"$EDGE_LOG_FILE" 2>&1 || true

    if ! wait "$RECEIVER_PID"; then
      echo "Browser API capture did not arrive before timeout" >&2
      echo "  source_url=$SOURCE_URL" >&2
      echo "  capture_url=$CAPTURE_URL" >&2
      echo "  viewport=${VIEWPORT_WIDTH}x${VIEWPORT_HEIGHT}" >&2
      echo "  virtual_time_budget_ms=$VIRTUAL_TIME_BUDGET_MS" >&2
      echo "  receiver_timeout_seconds=$RECEIVER_TIMEOUT_SECONDS" >&2
      echo "  outdir=$OUTDIR" >&2
      if [[ -s "$EDGE_LOG_FILE" ]]; then
        echo "  edge_log_tail:" >&2
        tail -n "$EDGE_LOG_TAIL_LINES" "$EDGE_LOG_FILE" >&2
      fi
      exit 1
    fi

    if [[ ! -s "$RESULT_FILE" ]]; then
      echo "Browser API capture receiver completed without an image path" >&2
      echo "  source_url=$SOURCE_URL" >&2
      echo "  capture_url=$CAPTURE_URL" >&2
      echo "  viewport=${VIEWPORT_WIDTH}x${VIEWPORT_HEIGHT}" >&2
      echo "  virtual_time_budget_ms=$VIRTUAL_TIME_BUDGET_MS" >&2
      echo "  receiver_timeout_seconds=$RECEIVER_TIMEOUT_SECONDS" >&2
      echo "  outdir=$OUTDIR" >&2
      exit 1
    fi

    printf '%s\n' "$(<"$RESULT_FILE")"
    ;;
  *)
    echo "Unsupported mode: $MODE" >&2
    usage >&2
    exit 1
    ;;
esac
