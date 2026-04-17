// Command-line compression benchmark runner for Android.
// Usage: ./pso_benchmark [--json output.json] [--csv output.csv] [--graphics]

#include "orchestrator/benchmark_orchestrator.h"
#include <fstream>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string json_path, csv_path;
    bool show_graphics = false;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--json" && i + 1 < argc) {
            json_path = argv[++i];
        } else if (std::string(argv[i]) == "--csv" && i + 1 < argc) {
            csv_path = argv[++i];
        } else if (std::string(argv[i]) == "--graphics") {
            show_graphics = true;
        }
    }

    benchmark::OrchestratorConfig config;
    config.enable_graphics = show_graphics;
    config.graphics_iterations = 3;
    config.compression_iterations = 5;

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    if (!report.graphics_matrix_text.empty()) {
        std::cout << report.graphics_matrix_text << "\n\n";
    }
    if (!report.compression_matrix_text.empty()) {
        std::cout << report.compression_matrix_text << "\n\n";
    }
    std::cout << report.summary_text << "\n";

    if (!json_path.empty()) {
        std::ofstream f(json_path);
        f << report.full_json;
        std::cout << "JSON written to: " << json_path << "\n";
    }
    if (!csv_path.empty() && !report.compression_csv.empty()) {
        std::ofstream f(csv_path);
        f << report.compression_csv;
        std::cout << "CSV written to: " << csv_path << "\n";
    }

    return 0;
}
