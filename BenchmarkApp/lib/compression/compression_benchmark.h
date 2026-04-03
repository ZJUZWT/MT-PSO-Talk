#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "compression/algorithms/compression_algorithm.h"
#include "results/schema/result_types.h"

namespace benchmark {

struct CompressionBenchmarkConfig {
    int iterations = 5;
    std::vector<int64_t> payload_sizes = {64 * 1024, 256 * 1024, 1024 * 1024};  // 64K, 256K, 1M
};

class CompressionBenchmark {
public:
    // Run all built-in algorithms across all configured payload sizes and iterations.
    std::vector<CompressionResult> run_all(
        const std::string& platform,
        const CompressionBenchmarkConfig& config = {});

    // Run a single compression benchmark for one algorithm with one payload.
    CompressionResult run_single(
        const std::string& platform,
        CompressionAlgorithm& algo,
        const std::string& level,
        const std::vector<uint8_t>& payload);
};

}  // namespace benchmark
