// Command-line benchmark runner for Android
// Usage: ./pso_benchmark [--json output.json] [--csv output.csv] [--mode vulkan|opengl|all]
// Runs on device via: adb push + adb shell

#include "orchestrator/benchmark_orchestrator.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    // Parse args
    std::string json_path, csv_path;
    std::string mode = "all";

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--json" && i+1 < argc) json_path = argv[++i];
        else if (std::string(argv[i]) == "--csv" && i+1 < argc) csv_path = argv[++i];
        else if (std::string(argv[i]) == "--mode" && i+1 < argc) mode = argv[++i];
    }

    benchmark::OrchestratorConfig config;
    config.graphics_iterations = 3;
    config.compression_iterations = 5;
    // Device info will be auto-detected

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    // Print to stdout
    std::cout << report.graphics_matrix_text << "\n\n";
    std::cout << report.compression_matrix_text << "\n\n";
    std::cout << report.summary_text << "\n";

    // Write files if requested
    if (!json_path.empty()) {
        std::ofstream f(json_path);
        f << report.full_json;
        std::cout << "JSON written to: " << json_path << "\n";
    }
    if (!csv_path.empty()) {
        std::ofstream f(csv_path);
        f << report.graphics_csv << "\n" << report.compression_csv;
        std::cout << "CSV written to: " << csv_path << "\n";
    }

    return 0;
}
