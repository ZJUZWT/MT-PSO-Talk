#!/bin/bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
APP_ROOT=$(cd "$SCRIPT_DIR/../.." && pwd)
SOURCE_PATH="$APP_ROOT/platform/ios/benchmark_app.mm"

if ! rg -q "@property \\(nonatomic, assign\\) BOOL didAutoStartBenchmark;" "$SOURCE_PATH"; then
  echo "Expected iOS benchmark view controller to track whether auto-run already happened"
  exit 1
fi

if ! rg -q -- "- \\(void\\)viewDidAppear:\\(BOOL\\)animated" "$SOURCE_PATH"; then
  echo "Expected iOS benchmark view controller to auto-start from viewDidAppear"
  exit 1
fi

if ! rg -q "if \\(!self\\.didAutoStartBenchmark\\)" "$SOURCE_PATH"; then
  echo "Expected iOS benchmark view controller to guard auto-run so it only triggers once"
  exit 1
fi

if ! rg -q "\\[self runBenchmark\\];" "$SOURCE_PATH"; then
  echo "Expected iOS benchmark view controller to invoke runBenchmark automatically"
  exit 1
fi
