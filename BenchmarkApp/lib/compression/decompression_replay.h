#pragma once

#include <cstdint>
#include <vector>

#include "compression/baseline_manifest.h"
#include "results/schema/result_types.h"

namespace benchmark {

// Legacy simulated replay (kept for backward compatibility)
CompressionResult simulate_replay(
    const CompressionBaselineManifest& baseline,
    int payload_size);

// Real replay: generates a payload, compresses/decompresses using the built-in
// algorithm that corresponds to the baseline manifest, and returns measured results.
CompressionResult real_replay(
    const CompressionBaselineManifest& baseline,
    const std::vector<uint8_t>& payload);

}  // namespace benchmark
