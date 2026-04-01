#include "Compression/decompression_replay.h"

#include "Compression/compression_job.h"

namespace benchmark {

CompressionResult simulate_replay(
    const CompressionBaselineManifest& baseline,
    int payload_size) {
    const CompressionJob job = make_replay_job(baseline, payload_size);

    CompressionResult result{};
    result.platform = "Simulated";
    result.algorithm = job.baseline.algorithm;
    result.baseline_package_id = job.baseline.baseline_package_id;
    result.decompressed_output_hash = job.baseline.decompressed_hash;
    result.decompress_us = payload_size >= 0 ? payload_size / 1024 : 0;
    return result;
}

}  // namespace benchmark
