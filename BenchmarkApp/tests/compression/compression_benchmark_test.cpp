#include "compression/compression_benchmark.h"
#include "compression/payload_generator.h"
#include "test_support/assert.h"

#include <iostream>
#include <set>

namespace {

class CorruptingAlgorithm final : public benchmark::CompressionAlgorithm {
public:
    std::string name() const override { return "corrupting"; }
    std::string version() const override { return "0.0.1"; }

    benchmark::CompressedPayload compress(
        const std::vector<uint8_t>& input,
        const std::string&) override {
        return {input, "corrupting", benchmark::compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const benchmark::CompressedPayload& payload) override {
        auto output = payload.data;
        if (!output.empty()) {
            output.back() ^= 0xFF;
        }
        return output;
    }

    std::vector<std::string> supported_levels() const override {
        return {"0"};
    }
};

size_t expected_result_count(int iterations, int payload_sizes, int payload_profiles) {
    size_t level_count = 14;
#if BENCHMARK_HAS_OODLE
    level_count = 22;
#endif
    return level_count * static_cast<size_t>(iterations) *
           static_cast<size_t>(payload_sizes) *
           static_cast<size_t>(payload_profiles);
}

}  // namespace

int main() {
    benchmark::CompressionBenchmark bench;

    // Test run_single with noop
    {
        auto payload = benchmark::generate_pso_cache_payload(4096);

        auto noop = benchmark::make_noop_algorithm();
        auto result = bench.run_single("TestPlatform", *noop, "0", payload);

        test_support::expect_equal(result.platform, std::string("TestPlatform"), "single platform");
        test_support::expect_equal(result.algorithm, std::string("noop"), "single algorithm");
        test_support::expect_equal(result.version, std::string("1.0.0"), "single version");
        test_support::expect_equal(result.level, std::string("0"), "single level");
        test_support::expect_equal(result.input_size, static_cast<int64_t>(4096), "single input_size");
        test_support::expect_equal(result.compressed_size, static_cast<int64_t>(4096), "noop compressed_size == input");
        test_support::expect_true(result.compression_ratio > 0.99, "noop ratio ~1.0");
        test_support::expect_true(result.compress_us >= 0, "compress timing");
        test_support::expect_true(result.decompress_us >= 0, "decompress timing");
        test_support::expect_equal(result.payload_profile, std::string("custom"), "single payload_profile");
        test_support::expect_equal(result.iteration_index, 0, "single iteration index");
        test_support::expect_true(!result.input_hash.empty(), "single input hash set");
        test_support::expect_true(!result.compressed_output_hash.empty(), "compressed hash set");
        test_support::expect_true(!result.decompressed_output_hash.empty(), "decompressed hash set");
        test_support::expect_true(result.roundtrip_hash_match, "single hash roundtrip match");
        test_support::expect_true(result.roundtrip_byte_match, "single byte roundtrip match");
        test_support::expect_equal(result.status, std::string("passed"), "single status passed");
    }

    // Test run_single with a real algorithm (zstd) — thorough checks
    {
        auto payload = benchmark::generate_pso_cache_payload(8192);
        auto zstd = benchmark::make_zstd_algorithm();
        auto result = bench.run_single("TestPlatform", *zstd, "3", payload);

        test_support::expect_equal(result.status, std::string("passed"), "zstd roundtrip passed");
        test_support::expect_equal(result.input_size, static_cast<int64_t>(8192), "zstd input size");
        test_support::expect_true(result.compressed_size < result.input_size, "zstd compressed_size < input_size");
        test_support::expect_greater_than_double(result.compression_ratio, 1.0, "zstd ratio > 1");
        test_support::expect_greater_than(result.compress_us, static_cast<int64_t>(0), "zstd compress_us > 0");
        test_support::expect_true(result.decompress_us >= 0, "zstd decompress_us >= 0");
        test_support::expect_equal(result.payload_profile, std::string("custom"), "zstd payload_profile");
        test_support::expect_equal(result.iteration_index, 0, "zstd iteration index");
        test_support::expect_true(!result.input_hash.empty(), "zstd input hash");
        test_support::expect_true(!result.compressed_output_hash.empty(), "zstd compressed hash");
        test_support::expect_true(!result.decompressed_output_hash.empty(), "zstd decompressed hash");
        test_support::expect_true(result.roundtrip_hash_match, "zstd hash roundtrip match");
        test_support::expect_true(result.roundtrip_byte_match, "zstd byte roundtrip match");
    }

    // Test failed roundtrip records explicit verification flags
    {
        auto payload = benchmark::generate_pso_cache_payload(512);
        CorruptingAlgorithm corrupting;
        auto result = bench.run_single("TestPlatform", corrupting, "0", payload);

        test_support::expect_equal(result.status, std::string("failed"), "corrupting status failed");
        test_support::expect_true(!result.roundtrip_hash_match, "corrupting hash mismatch");
        test_support::expect_true(!result.roundtrip_byte_match, "corrupting byte mismatch");
    }

    // Test with iterations=3 and 3 payload profiles
    {
        benchmark::CompressionBenchmarkConfig config;
        config.iterations = 3;
        config.payload_sizes = {4096};
        config.payload_profiles = {"pso_like", "high_compressibility", "low_compressibility"};

        auto results = bench.run_all("IterPlatform", config);

        size_t expected_count = expected_result_count(/*iterations=*/3, /*payload_sizes=*/1, /*payload_profiles=*/3);
        test_support::expect_equal(results.size(), expected_count,
            "iterations=3 result count");
    }

    // Test run_all with small config
    {
        benchmark::CompressionBenchmarkConfig config;
        config.iterations = 1;
        config.payload_sizes = {4096};
        config.payload_profiles = {"pso_like", "high_compressibility", "low_compressibility"};

        auto results = bench.run_all("BenchPlatform", config);

        size_t expected_count = expected_result_count(/*iterations=*/1, /*payload_sizes=*/1, /*payload_profiles=*/3);
        test_support::expect_equal(results.size(), expected_count,
            "run_all result count");

        // All should pass
        for (const auto& r : results) {
            test_support::expect_equal(r.status, std::string("passed"),
                "status for " + r.algorithm + " level=" + r.level);
        }

        // Check that we have all algorithm names
        std::set<std::string> algo_names;
        for (const auto& r : results) {
            algo_names.insert(r.algorithm);
        }
        size_t expected_algorithms = 6;
#if BENCHMARK_HAS_OODLE
        expected_algorithms = 10;
#endif
        test_support::expect_equal(algo_names.size(), expected_algorithms, "unique algorithms");
        test_support::expect_true(algo_names.count("noop") > 0, "has noop results");
        test_support::expect_true(algo_names.count("zstd") > 0, "has zstd results");
        test_support::expect_true(algo_names.count("lz4") > 0, "has lz4 results");
        test_support::expect_true(algo_names.count("zlib") > 0, "has zlib results");
        test_support::expect_true(algo_names.count("snappy") > 0, "has snappy results");
        test_support::expect_true(algo_names.count("brotli") > 0, "has brotli results");
#if BENCHMARK_HAS_OODLE
        test_support::expect_true(algo_names.count("oodle_kraken") > 0, "has oodle_kraken results");
        test_support::expect_true(algo_names.count("oodle_leviathan") > 0, "has oodle_leviathan results");
        test_support::expect_true(algo_names.count("oodle_mermaid") > 0, "has oodle_mermaid results");
        test_support::expect_true(algo_names.count("oodle_selkie") > 0, "has oodle_selkie results");
#endif

        std::set<std::string> payload_profiles;
        for (const auto& r : results) {
            payload_profiles.insert(r.payload_profile);
        }
        test_support::expect_equal(payload_profiles.size(), static_cast<size_t>(3), "unique payload profiles");
        test_support::expect_true(payload_profiles.count("pso_like") > 0, "has pso_like payload profile");
        test_support::expect_true(payload_profiles.count("high_compressibility") > 0, "has high payload profile");
        test_support::expect_true(payload_profiles.count("low_compressibility") > 0, "has low payload profile");

        // Check that all results have valid fields and real algos compress
        for (const auto& r : results) {
            test_support::expect_equal(r.platform, std::string("BenchPlatform"), "platform set");
            test_support::expect_true(!r.algorithm.empty(), "algorithm set");
            test_support::expect_true(!r.version.empty(), "version set");
            test_support::expect_true(!r.payload_profile.empty(), "payload_profile set");
            test_support::expect_equal(r.iteration_index, 0, "single-iteration index");
            test_support::expect_greater_than(r.input_size, static_cast<int64_t>(0), "input_size > 0");
            test_support::expect_greater_than(r.compressed_size, static_cast<int64_t>(0), "compressed_size > 0");
            test_support::expect_greater_than_double(r.compression_ratio, 0.0, "ratio > 0");
            test_support::expect_true(!r.input_hash.empty(), "input hash");
            test_support::expect_true(!r.compressed_output_hash.empty(), "compressed hash");
            test_support::expect_true(!r.decompressed_output_hash.empty(), "decompressed hash");
            test_support::expect_true(!r.baseline_package_id.empty(), "baseline_package_id set");
            test_support::expect_true(r.roundtrip_hash_match, "roundtrip hash match");
            test_support::expect_true(r.roundtrip_byte_match, "roundtrip byte match");

            // Real algorithms (not noop) must produce valid timing data.
            // Extremely fast decompression can legitimately round down to 0us.
            // Some payload profiles are intentionally low-compressibility, so
            // expansion is allowed as long as roundtrip correctness holds.
            if (r.algorithm != "noop") {
                test_support::expect_greater_than(r.compress_us, static_cast<int64_t>(0),
                    r.algorithm + " compress_us > 0");
                test_support::expect_true(r.decompress_us >= 0,
                    r.algorithm + " decompress_us >= 0");
            }
        }
    }

    std::cout << "compression_benchmark_test: all checks completed\n";
    return test_support::finish();
}
