#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "compression/baseline_manifest.h"

namespace benchmark {

struct CompressionJob {
    CompressionBaselineManifest baseline;
    int payload_size = 0;
    std::vector<uint8_t> payload_data;  // optional: actual payload for real benchmarks
    int iterations = 1;
};

CompressionJob make_replay_job(
    const CompressionBaselineManifest& baseline,
    int payload_size);

CompressionJob make_benchmark_job(
    const CompressionBaselineManifest& baseline,
    int payload_size,
    int iterations);

// Create a job with actual payload data for real compression benchmarks
CompressionJob make_real_replay_job(
    const CompressionBaselineManifest& baseline,
    std::vector<uint8_t> payload);

}  // namespace benchmark
