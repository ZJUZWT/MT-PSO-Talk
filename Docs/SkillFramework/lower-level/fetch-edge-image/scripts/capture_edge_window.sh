#!/usr/bin/env bash

set -euo pipefail

usage() {
  cat <<'EOF'
Usage: capture_edge_window.sh [--outdir PATH] [--prefix NAME] [--mode auto|screenshot]

Captures the front Microsoft Edge tab as a local image artifact.

Defaults:
  --outdir  <cwd>/ignore/edge-captures/<timestamp>/
  --prefix  edge-capture
  --mode    auto
EOF
}

OUTDIR=""
PREFIX="edge-capture"
MODE="auto"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --outdir)
      OUTDIR="$2"
      shift 2
      ;;
    --prefix)
      PREFIX="$2"
      shift 2
      ;;
    --mode)
      MODE="$2"
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

if [[ "$MODE" != "auto" && "$MODE" != "screenshot" ]]; then
  echo "Unsupported mode: $MODE" >&2
  exit 1
fi

timestamp="$(date +%Y%m%d-%H%M%S)"

if [[ -z "$OUTDIR" ]]; then
  if [[ -d "$PWD/ignore" || ! -e "$PWD/ignore" ]]; then
    OUTDIR="$PWD/ignore/edge-captures/$timestamp"
  else
    OUTDIR="$PWD/.generated/edge-captures/$timestamp"
  fi
fi

mkdir -p "$OUTDIR"

read_edge_value() {
  osascript -e "$1"
}

TITLE="$(read_edge_value 'tell application "Microsoft Edge" to get title of active tab of front window')"
URL="$(read_edge_value 'tell application "Microsoft Edge" to get URL of active tab of front window')"
BOUNDS_RAW="$(read_edge_value 'tell application "Microsoft Edge" to get bounds of front window')"

IFS=', ' read -r LEFT TOP RIGHT BOTTOM <<< "$BOUNDS_RAW"
WIDTH=$((RIGHT - LEFT))
HEIGHT=$((BOTTOM - TOP))

MODE_USED=""
IMAGE_PATH=""

if [[ "$MODE" == "auto" ]]; then
  case "${URL%%\?*}" in
    *.png) EXT="png" ;;
    *.jpg|*.jpeg) EXT="jpg" ;;
    *.webp) EXT="webp" ;;
    *.gif) EXT="gif" ;;
    *.svg) EXT="svg" ;;
    *) EXT="" ;;
  esac

  if [[ -n "${EXT:-}" ]]; then
    IMAGE_PATH="$OUTDIR/${PREFIX}-asset.$EXT"
    if curl -fsSL "$URL" -o "$IMAGE_PATH"; then
      MODE_USED="direct-download"
    else
      IMAGE_PATH=""
      MODE_USED="screenshot-fallback"
    fi
  fi
fi

if [[ "$MODE" == "screenshot" || "$MODE_USED" != "direct-download" ]]; then
  IMAGE_PATH="$OUTDIR/${PREFIX}.png"
  screencapture -x -R "${LEFT},${TOP},${WIDTH},${HEIGHT}" "$IMAGE_PATH"
  if [[ -z "$MODE_USED" || "$MODE_USED" == "screenshot-fallback" ]]; then
    MODE_USED="${MODE_USED:-screenshot}"
  fi
fi

META_PATH="$OUTDIR/${PREFIX}.txt"
cat >"$META_PATH" <<EOF
mode=$MODE_USED
title=$TITLE
url=$URL
bounds_css=$LEFT,$TOP,$RIGHT,$BOTTOM
capture_region_css=$LEFT,$TOP,$WIDTH,$HEIGHT
image_path=$IMAGE_PATH
note=device-pixel output may be larger than CSS bounds on Retina displays
EOF

printf '%s\n' "$IMAGE_PATH"
