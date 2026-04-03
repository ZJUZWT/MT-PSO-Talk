#!/bin/bash
# Run PSO benchmark on Android device
# Prerequisites: adb connected, NDK cross-compiled binary

set -e

BINARY=${1:-"BenchmarkApp/build-android/pso_benchmark"}
DEVICE_DIR="/data/local/tmp/pso_benchmark"
OUTPUT_DIR="benchmark_results"

echo "=== PSO Benchmark Runner ==="

# Create device directory
adb shell "mkdir -p $DEVICE_DIR"

# Push binary and shader assets
echo "Pushing binary..."
adb push "$BINARY" "$DEVICE_DIR/pso_benchmark"
adb shell "chmod +x $DEVICE_DIR/pso_benchmark"

# Run benchmark
echo "Running benchmark..."
adb shell "cd $DEVICE_DIR && \
    MESA_SHADER_CACHE_DISABLE=true \
    ./pso_benchmark --json results.json --csv results.csv" 2>&1 | tee /dev/stderr

# Pull results
mkdir -p "$OUTPUT_DIR"
echo "Pulling results..."
adb pull "$DEVICE_DIR/results.json" "$OUTPUT_DIR/"
adb pull "$DEVICE_DIR/results.csv" "$OUTPUT_DIR/"

echo ""
echo "=== Results saved to $OUTPUT_DIR/ ==="
echo "  JSON: $OUTPUT_DIR/results.json"
echo "  CSV:  $OUTPUT_DIR/results.csv"
