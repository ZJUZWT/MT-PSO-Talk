#include "compression/compression_job.h"

#include <utility>

namespace benchmark {

CompressionJob make_replay_job(
    const CompressionBaselineManifest& baseline,
    int payload_size) {
    CompressionJob job;
    job.baseline = baseline;
    job.payload_size = payload_size;
    job.iterations = 1;
    return job;
}

CompressionJob make_benchmark_job(
    const CompressionBaselineManifest& baseline,
    int payload_size,
    int iterations) {
    CompressionJob job;
    job.baseline = baseline;
    job.payload_size = payload_size;
    job.iterations = iterations;
    return job;
}

CompressionJob make_real_replay_job(
    const CompressionBaselineManifest& baseline,
    std::vector<uint8_t> payload) {
    CompressionJob job;
    job.baseline = baseline;
    job.payload_size = static_cast<int>(payload.size());
    job.payload_data = std::move(payload);
    job.iterations = 1;
    return job;
}

}  // namespace benchmark
