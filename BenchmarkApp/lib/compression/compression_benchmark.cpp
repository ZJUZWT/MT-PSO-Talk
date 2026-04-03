#include "compression/compression_benchmark.h"

#include <functional>
#include <memory>

#include "compression/payload_generator.h"
#include "driver/common/timing_scope.h"

namespace benchmark {

CompressionResult CompressionBenchmark::run_single(
    const std::string& platform,
    CompressionAlgorithm& algo,
    const std::string& level,
    const std::vector<uint8_t>& payload) {

    CompressionResult result{};
    result.platform = platform;
    result.algorithm = algo.name();
    result.version = algo.version();
    result.level = level;
    result.input_size = static_cast<int64_t>(payload.size());

    // Compute input hash
    std::string input_hash = compute_hash(payload);

    // Time compression
    CompressedPayload compressed;
    {
        TimingScope timer(result.compress_us);
        compressed = algo.compress(payload, level);
    }

    result.compressed_size = static_cast<int64_t>(compressed.data.size());
    result.compression_ratio = (result.input_size > 0)
        ? static_cast<double>(result.input_size) / static_cast<double>(result.compressed_size)
        : 0.0;
    result.compressed_output_hash = compute_hash(compressed.data);

    // Time decompression
    std::vector<uint8_t> decompressed;
    {
        TimingScope timer(result.decompress_us);
        decompressed = algo.decompress(compressed);
    }

    result.decompressed_output_hash = compute_hash(decompressed);

    // Verify roundtrip correctness
    if (decompressed.size() != payload.size()) {
        result.status = "failed";
    } else if (result.decompressed_output_hash != input_hash) {
        result.status = "failed";
    } else {
        result.status = "passed";
    }

    // Calculate throughput (MB/s) based on compression time
    if (result.compress_us > 0) {
        double seconds = static_cast<double>(result.compress_us) / 1'000'000.0;
        double megabytes = static_cast<double>(result.input_size) / (1024.0 * 1024.0);
        result.throughput_mbps = megabytes / seconds;
    }

    return result;
}

std::vector<CompressionResult> CompressionBenchmark::run_all(
    const std::string& platform,
    const CompressionBenchmarkConfig& config) {

    auto& registry = CompressionRegistry::instance();
    std::vector<CompressionResult> results;

    for (auto& entry : registry.all()) {
        auto algo = entry.factory();
        auto levels = algo->supported_levels();

        for (auto& level : levels) {
            for (auto payload_size : config.payload_sizes) {
                auto payload = generate_pso_cache_payload(payload_size);

                for (int iter = 0; iter < config.iterations; ++iter) {
                    auto result = run_single(platform, *algo, level, payload);
                    result.baseline_package_id =
                        "pso-cache-" + algo->name() + "-v1";
                    results.push_back(std::move(result));
                }
            }
        }
    }

    return results;
}

}  // namespace benchmark
