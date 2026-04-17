#include <iostream>
#include <set>
#include <string>

#include "orchestrator/benchmark_orchestrator.h"
#include "test_support/assert.h"

int main() {
    // Run orchestrator once with known config for all tests
    benchmark::OrchestratorConfig config;
    config.compression_iterations = 1;

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    // Product default is now compression-only.
    test_support::expect_true(report.graphics_results.empty(),
        "graphics results disabled by default");

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
#if BENCHMARK_HAS_OODLE
        test_support::expect_true(algo_names.count("oodle_kraken") > 0, "compression has oodle_kraken");
        test_support::expect_true(algo_names.count("oodle_leviathan") > 0, "compression has oodle_leviathan");
        test_support::expect_true(algo_names.count("oodle_mermaid") > 0, "compression has oodle_mermaid");
        test_support::expect_true(algo_names.count("oodle_selkie") > 0, "compression has oodle_selkie");
#endif
    }

    // Verify all compression results have status == "passed"
    for (const auto& r : report.compression_results) {
        test_support::expect_equal(r.status, std::string("passed"),
            "compression " + r.algorithm + " status");
    }

    // Default product output should not include graphics tables.
    test_support::expect_true(report.graphics_matrix_text.empty(), "graphics matrix empty by default");

    // Verify compression_matrix_text contains all algorithm names
    test_support::expect_true(!report.compression_matrix_text.empty(), "compression matrix non-empty");
    test_support::expect_contains(report.compression_matrix_text, "zstd", "comp matrix has zstd");
    test_support::expect_contains(report.compression_matrix_text, "lz4", "comp matrix has lz4");
    test_support::expect_contains(report.compression_matrix_text, "zlib", "comp matrix has zlib");
    test_support::expect_contains(report.compression_matrix_text, "snappy", "comp matrix has snappy");
    test_support::expect_contains(report.compression_matrix_text, "brotli", "comp matrix has brotli");
    test_support::expect_contains(report.compression_matrix_text, "Ratio", "comp matrix has Ratio");

    // Verify summary_text contains key stats and is compression-only by default.
    test_support::expect_true(!report.summary_text.empty(), "summary non-empty");
    test_support::expect_not_contains(report.summary_text, "Graphics:", "summary omits graphics section");
    test_support::expect_contains(report.summary_text, "Best ratio", "summary has Best ratio");
    test_support::expect_contains(report.summary_text, "Fastest decompress", "summary has Fastest decompress");

    // Verify full_json contains "benchmark" and all algorithm names.
    test_support::expect_true(!report.full_json.empty(), "full json non-empty");
    test_support::expect_contains(report.full_json, "\"benchmark\"", "json has benchmark");
    test_support::expect_contains(report.full_json, "\"graphics\"", "json has graphics");
    test_support::expect_contains(report.full_json, "\"compression\"", "json has compression");
    test_support::expect_contains(report.full_json, "\"device\"", "json has device");
    test_support::expect_contains(report.full_json, "zstd", "json has zstd");
    test_support::expect_contains(report.full_json, "lz4", "json has lz4");
    test_support::expect_contains(report.full_json, "snappy", "json has snappy");

    // Verify CSV output
    test_support::expect_true(report.graphics_csv.empty(), "graphics csv empty by default");
    test_support::expect_true(!report.compression_csv.empty(), "compression csv non-empty");
    test_support::expect_contains(report.compression_csv, "platform,algorithm", "compression csv header");

    std::cout << "orchestrator_test: all checks completed\n";
    return test_support::finish();
}
