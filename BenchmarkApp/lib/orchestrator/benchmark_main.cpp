#include <cstring>
#include <fstream>
#include <iostream>

#include "orchestrator/benchmark_orchestrator.h"

int main(int argc, char* argv[]) {
    benchmark::OrchestratorConfig config;

    // Parse optional flags
    bool write_json = false;
    bool write_csv = false;
    std::string json_path = "benchmark_report.json";
    std::string csv_graphics_path = "graphics_results.csv";
    std::string csv_compression_path = "compression_results.csv";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--json") == 0) {
            write_json = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                json_path = argv[++i];
            }
        } else if (std::strcmp(argv[i], "--csv") == 0) {
            write_csv = true;
        }
    }

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    // Print matrix reports
    std::cout << report.graphics_matrix_text << "\n";
    std::cout << report.compression_matrix_text << "\n";
    std::cout << report.summary_text << "\n";

    // Optionally write files
    if (write_json) {
        std::ofstream ofs(json_path);
        ofs << report.full_json;
        std::cout << "JSON written to " << json_path << "\n";
    }

    if (write_csv) {
        {
            std::ofstream ofs(csv_graphics_path);
            ofs << report.graphics_csv;
            std::cout << "Graphics CSV written to " << csv_graphics_path << "\n";
        }
        {
            std::ofstream ofs(csv_compression_path);
            ofs << report.compression_csv;
            std::cout << "Compression CSV written to " << csv_compression_path << "\n";
        }
    }

    return 0;
}
