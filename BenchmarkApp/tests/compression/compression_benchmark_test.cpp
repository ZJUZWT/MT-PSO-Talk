#include "compression/compression_benchmark.h"
#include "compression/payload_generator.h"
#include "test_support/assert.h"

#include <iostream>
#include <set>

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
        test_support::expect_true(!result.compressed_output_hash.empty(), "compressed hash set");
        test_support::expect_true(!result.decompressed_output_hash.empty(), "decompressed hash set");
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
        test_support::expect_greater_than(result.decompress_us, static_cast<int64_t>(0), "zstd decompress_us > 0");
        test_support::expect_true(!result.compressed_output_hash.empty(), "zstd compressed hash");
        test_support::expect_true(!result.decompressed_output_hash.empty(), "zstd decompressed hash");
    }

    // Test with iterations=3, verify we get 3x results
    {
        benchmark::CompressionBenchmarkConfig config;
        config.iterations = 3;
        config.payload_sizes = {4096};

        auto results = bench.run_all("IterPlatform", config);

        // Expected: 25 algo/level combos * 1 size * 3 iterations = 75
        size_t expected_count = 75;
        test_support::expect_equal(results.size(), expected_count,
            "iterations=3 result count");
    }

    // Test run_all with small config
    {
        benchmark::CompressionBenchmarkConfig config;
        config.iterations = 1;
        config.payload_sizes = {4096};

        auto results = bench.run_all("BenchPlatform", config);

        // Expected entries:
        // noop(1) + zstd(4) + lz4(2) + zlib(3) + snappy(1) + brotli(3) + lzma(3)
        // + oodle_kraken(2) + oodle_leviathan(2) + oodle_mermaid(2) + oodle_selkie(2) = 25
        // 25 algos * 1 size * 1 iteration = 25
        size_t expected_count = 25;
        test_support::expect_equal(results.size(), expected_count,
            "run_all result count");

        // All should pass
        for (const auto& r : results) {
            test_support::expect_equal(r.status, std::string("passed"),
                "status for " + r.algorithm + " level=" + r.level);
        }

        // Check that we have all 11 algorithm names
        std::set<std::string> algo_names;
        for (const auto& r : results) {
            algo_names.insert(r.algorithm);
        }
        test_support::expect_equal(algo_names.size(), static_cast<size_t>(11), "11 unique algorithms");
        test_support::expect_true(algo_names.count("noop") > 0, "has noop results");
        test_support::expect_true(algo_names.count("zstd") > 0, "has zstd results");
        test_support::expect_true(algo_names.count("lz4") > 0, "has lz4 results");
        test_support::expect_true(algo_names.count("zlib") > 0, "has zlib results");
        test_support::expect_true(algo_names.count("snappy") > 0, "has snappy results");
        test_support::expect_true(algo_names.count("brotli") > 0, "has brotli results");
        test_support::expect_true(algo_names.count("lzma") > 0, "has lzma results");
        test_support::expect_true(algo_names.count("oodle_kraken") > 0, "has oodle_kraken results");
        test_support::expect_true(algo_names.count("oodle_leviathan") > 0, "has oodle_leviathan results");
        test_support::expect_true(algo_names.count("oodle_mermaid") > 0, "has oodle_mermaid results");
        test_support::expect_true(algo_names.count("oodle_selkie") > 0, "has oodle_selkie results");

        // Check that all results have valid fields and real algos compress
        for (const auto& r : results) {
            test_support::expect_equal(r.platform, std::string("BenchPlatform"), "platform set");
            test_support::expect_true(!r.algorithm.empty(), "algorithm set");
            test_support::expect_true(!r.version.empty(), "version set");
            test_support::expect_greater_than(r.input_size, static_cast<int64_t>(0), "input_size > 0");
            test_support::expect_greater_than(r.compressed_size, static_cast<int64_t>(0), "compressed_size > 0");
            test_support::expect_greater_than_double(r.compression_ratio, 0.0, "ratio > 0");
            test_support::expect_true(!r.compressed_output_hash.empty(), "compressed hash");
            test_support::expect_true(!r.decompressed_output_hash.empty(), "decompressed hash");
            test_support::expect_true(!r.baseline_package_id.empty(), "baseline_package_id set");

            // Real algorithms (not noop) should actually compress
            if (r.algorithm != "noop") {
                test_support::expect_true(r.compressed_size < r.input_size,
                    r.algorithm + " compressed_size < input_size");
                test_support::expect_greater_than(r.compress_us, static_cast<int64_t>(0),
                    r.algorithm + " compress_us > 0");
                test_support::expect_greater_than(r.decompress_us, static_cast<int64_t>(0),
                    r.algorithm + " decompress_us > 0");
            }
        }
    }

    std::cout << "compression_benchmark_test: all checks completed\n";
    return test_support::finish();
}
