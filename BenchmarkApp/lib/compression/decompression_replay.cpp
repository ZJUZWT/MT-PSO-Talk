#include "compression/decompression_replay.h"

#include <memory>

#include "compression/algorithms/compression_algorithm.h"
#include "compression/compression_benchmark.h"
#include "compression/compression_job.h"

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

CompressionResult real_replay(
    const CompressionBaselineManifest& baseline,
    const std::vector<uint8_t>& payload) {

    // Map baseline algorithm name to a built-in algorithm.
    // The built-in algorithms stand in for the real external libraries.
    std::unique_ptr<CompressionAlgorithm> algo;
    if (baseline.algorithm == "lz4") {
        algo = make_rle_algorithm();        // RLE stands in for LZ4
    } else if (baseline.algorithm == "zstd") {
        algo = make_dictionary_algorithm(); // Dictionary stands in for zstd
    } else if (baseline.algorithm == "zlib") {
        algo = make_deflate_lite_algorithm(); // Deflate-lite stands in for zlib
    } else if (baseline.algorithm == "snappy") {
        algo = make_noop_algorithm();       // Noop stands in for snappy
    } else {
        algo = make_noop_algorithm();
    }

    CompressionBenchmark bench;
    auto result = bench.run_single("Native", *algo, baseline.level, payload);
    result.baseline_package_id = baseline.baseline_package_id;
    return result;
}

}  // namespace benchmark
