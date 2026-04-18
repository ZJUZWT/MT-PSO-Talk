#include "compression/compression_benchmark.h"

#include <functional>
#include <memory>
#include <stdexcept>

#include "compression/payload_generator.h"
#include "driver/common/timing_scope.h"

namespace benchmark {
namespace {

std::vector<uint8_t> generate_payload_for_profile(
    const std::string& payload_profile,
    int64_t payload_size) {
    if (payload_profile == "pso_like") {
        return generate_pso_cache_payload(payload_size);
    }
    if (payload_profile == "high_compressibility") {
        return generate_payload_with_compressibility(payload_size, 0.90);
    }
    if (payload_profile == "low_compressibility") {
        return generate_payload_with_compressibility(payload_size, 0.10);
    }

    throw std::invalid_argument("Unsupported compression payload profile: " + payload_profile);
}

}  // namespace

CompressionResult CompressionBenchmark::run_single(
    const std::string& platform,
    CompressionAlgorithm& algo,
    const std::string& level,
    const std::vector<uint8_t>& payload,
    const std::string& payload_profile,
    int iteration_index) {

    CompressionResult result{};
    result.platform = platform;
    result.algorithm = algo.name();
    result.version = algo.version();
    result.level = level;
    result.payload_profile = payload_profile;
    result.iteration_index = iteration_index;
    result.input_size = static_cast<int64_t>(payload.size());

    // Compute input hash
    std::string input_hash = compute_hash(payload);
    result.input_hash = input_hash;

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
    result.roundtrip_hash_match = (result.decompressed_output_hash == input_hash);
    result.roundtrip_byte_match = (decompressed == payload);

    // Verify roundtrip correctness
    result.status = (result.roundtrip_hash_match && result.roundtrip_byte_match)
        ? "passed"
        : "failed";

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
            for (const auto& payload_profile : config.payload_profiles) {
                for (auto payload_size : config.payload_sizes) {
                    auto payload = generate_payload_for_profile(payload_profile, payload_size);

                    for (int warmup = 0; warmup < config.warmup_iterations; ++warmup) {
                        (void)run_single(
                            platform,
                            *algo,
                            level,
                            payload,
                            payload_profile,
                            -1);
                    }

                    for (int iter = 0; iter < config.iterations; ++iter) {
                        auto result = run_single(
                            platform,
                            *algo,
                            level,
                            payload,
                            payload_profile,
                            iter);
                        result.baseline_package_id =
                            "compression-bench-" + algo->name() + "-" + level + "-" +
                            payload_profile + "-" + std::to_string(payload_size) + "-v1";
                        results.push_back(std::move(result));
                    }
                }
            }
        }
    }

    return results;
}

}  // namespace benchmark
