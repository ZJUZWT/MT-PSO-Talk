#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "results/schema/result_types.h"

namespace benchmark {

struct OrchestratorConfig {
    bool enable_graphics = false;
    bool enable_compression = true;
    int graphics_iterations = 2;
    bool graphics_cold_cache = true;
    bool graphics_warm_cache = true;
    int compression_iterations = 5;
    int compression_warmup_iterations = 1;
    std::vector<int64_t> compression_payload_sizes = {64 * 1024, 256 * 1024, 1024 * 1024};
    std::vector<std::string> compression_payload_profiles = {
        "pso_like",
        "high_compressibility",
        "low_compressibility"
    };
    std::string device_model;
    std::string soc;
    std::string os_version;
};

struct BenchmarkReport {
    std::vector<GraphicsResult> graphics_results;
    std::vector<CompressionResult> compression_results;

    // Formatted output
    std::string graphics_matrix_text;     // ASCII table
    std::string compression_matrix_text;  // ASCII table
    std::string full_json;                // Complete JSON report
    std::string graphics_csv;             // CSV with header
    std::string compression_csv;          // CSV with header
    std::string summary_text;             // Human-readable summary
};

class BenchmarkOrchestrator {
public:
    BenchmarkReport run(const OrchestratorConfig& config = {});

    // Individual report generators
    static std::string format_graphics_matrix(const std::vector<GraphicsResult>& results);
    static std::string format_compression_matrix(const std::vector<CompressionResult>& results);
    static std::string format_summary(const BenchmarkReport& report);
    static std::string format_full_json(const BenchmarkReport& report);
};

}  // namespace benchmark
