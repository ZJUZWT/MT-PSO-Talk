#include <iostream>
#include <set>
#include <string>

#include "orchestrator/benchmark_orchestrator.h"
#include "test_support/assert.h"

int main() {
    // Run orchestrator once with known config for all tests
    benchmark::OrchestratorConfig config;
    config.graphics_iterations = 2;
    config.compression_iterations = 1;

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    // Verify graphics_results.size() == 5 backends * 4 tiers * 2 cache * 2 iterations = 80
    test_support::expect_equal(report.graphics_results.size(), static_cast<size_t>(80),
        "graphics_results count (5*4*2*2=80)");

    // Verify compression_results is non-empty and has all algorithm names
    test_support::expect_true(!report.compression_results.empty(), "compression results non-empty");
    {
        std::set<std::string> algo_names;
        for (const auto& r : report.compression_results) {
            algo_names.insert(r.algorithm);
        }
        test_support::expect_true(algo_names.count("noop") > 0, "compression has noop");
        test_support::expect_true(algo_names.count("zstd") > 0, "compression has zstd");
        test_support::expect_true(algo_names.count("lz4") > 0, "compression has lz4");
        test_support::expect_true(algo_names.count("zlib") > 0, "compression has zlib");
        test_support::expect_true(algo_names.count("snappy") > 0, "compression has snappy");
        test_support::expect_true(algo_names.count("brotli") > 0, "compression has brotli");
        test_support::expect_true(algo_names.count("lzma") > 0, "compression has lzma");
        test_support::expect_true(algo_names.count("oodle_kraken") > 0, "compression has oodle_kraken");
        test_support::expect_true(algo_names.count("oodle_leviathan") > 0, "compression has oodle_leviathan");
        test_support::expect_true(algo_names.count("oodle_mermaid") > 0, "compression has oodle_mermaid");
        test_support::expect_true(algo_names.count("oodle_selkie") > 0, "compression has oodle_selkie");
    }

    // Verify all 5 backends present in graphics results
    {
        std::set<std::string> backends;
        for (const auto& r : report.graphics_results) {
            backends.insert(r.platform + " " + r.driver_mode + " " + r.api);
        }
        test_support::expect_equal(backends.size(), static_cast<size_t>(5), "5 unique backends");
        test_support::expect_true(backends.count("Android Mesa OpenGL") == 1, "has Mesa OpenGL");
        test_support::expect_true(backends.count("Android Mesa Vulkan") == 1, "has Mesa Vulkan");
        test_support::expect_true(backends.count("Android Native OpenGL") == 1, "has Native OpenGL");
        test_support::expect_true(backends.count("Android Native Vulkan") == 1, "has Native Vulkan");
        test_support::expect_true(backends.count("iOS Native Metal") == 1, "has Metal");
    }

    // Verify all graphics results have status == "passed"
    for (const auto& r : report.graphics_results) {
        test_support::expect_equal(r.status, std::string("passed"),
            "graphics " + r.api + " " + r.driver_mode + " status");
    }

    // Verify all compression results have status == "passed"
    for (const auto& r : report.compression_results) {
        test_support::expect_equal(r.status, std::string("passed"),
            "compression " + r.algorithm + " status");
    }

    // Verify graphics_matrix_text contains all 5 backend names
    test_support::expect_true(!report.graphics_matrix_text.empty(), "graphics matrix non-empty");
    test_support::expect_contains(report.graphics_matrix_text, "Mesa OpenGL", "matrix has Mesa OpenGL");
    test_support::expect_contains(report.graphics_matrix_text, "Mesa Vulkan", "matrix has Mesa Vulkan");
    test_support::expect_contains(report.graphics_matrix_text, "Native OpenGL", "matrix has Native OpenGL");
    test_support::expect_contains(report.graphics_matrix_text, "Native Vulkan", "matrix has Native Vulkan");
    test_support::expect_contains(report.graphics_matrix_text, "Metal", "matrix has Metal");
    test_support::expect_contains(report.graphics_matrix_text, "Cold", "matrix has Cold");
    test_support::expect_contains(report.graphics_matrix_text, "Warm", "matrix has Warm");

    // Verify compression_matrix_text contains all algorithm names
    test_support::expect_true(!report.compression_matrix_text.empty(), "compression matrix non-empty");
    test_support::expect_contains(report.compression_matrix_text, "zstd", "comp matrix has zstd");
    test_support::expect_contains(report.compression_matrix_text, "lz4", "comp matrix has lz4");
    test_support::expect_contains(report.compression_matrix_text, "zlib", "comp matrix has zlib");
    test_support::expect_contains(report.compression_matrix_text, "snappy", "comp matrix has snappy");
    test_support::expect_contains(report.compression_matrix_text, "brotli", "comp matrix has brotli");
    test_support::expect_contains(report.compression_matrix_text, "lzma", "comp matrix has lzma");
    test_support::expect_contains(report.compression_matrix_text, "Ratio", "comp matrix has Ratio");

    // Verify summary_text contains key stats
    test_support::expect_true(!report.summary_text.empty(), "summary non-empty");
    test_support::expect_contains(report.summary_text, "Fastest", "summary has Fastest");
    test_support::expect_contains(report.summary_text, "Slowest", "summary has Slowest");
    test_support::expect_contains(report.summary_text, "Best ratio", "summary has Best ratio");

    // Verify full_json contains "benchmark" and all algorithm names
    test_support::expect_true(!report.full_json.empty(), "full json non-empty");
    test_support::expect_contains(report.full_json, "\"benchmark\"", "json has benchmark");
    test_support::expect_contains(report.full_json, "\"graphics\"", "json has graphics");
    test_support::expect_contains(report.full_json, "\"compression\"", "json has compression");
    test_support::expect_contains(report.full_json, "\"device\"", "json has device");
    test_support::expect_contains(report.full_json, "zstd", "json has zstd");
    test_support::expect_contains(report.full_json, "lz4", "json has lz4");
    test_support::expect_contains(report.full_json, "snappy", "json has snappy");

    // Verify CSV output
    test_support::expect_true(!report.graphics_csv.empty(), "graphics csv non-empty");
    test_support::expect_true(!report.compression_csv.empty(), "compression csv non-empty");
    test_support::expect_contains(report.graphics_csv, "platform,api", "graphics csv header");
    test_support::expect_contains(report.compression_csv, "platform,algorithm", "compression csv header");

    std::cout << "orchestrator_test: all checks completed\n";
    return test_support::finish();
}
