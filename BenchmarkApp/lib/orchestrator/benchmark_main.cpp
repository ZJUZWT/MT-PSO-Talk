#include <cstring>
#include <fstream>
#include <iostream>

#include "orchestrator/benchmark_orchestrator.h"

int main(int argc, char* argv[]) {
    benchmark::OrchestratorConfig config;

    bool show_graphics = false;
    bool write_json = false;
    bool write_csv = false;
    std::string json_path = "benchmark_report.json";
    std::string csv_compression_path = "compression_results.csv";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--graphics") == 0) {
            show_graphics = true;
        } else if (std::strcmp(argv[i], "--json") == 0) {
            write_json = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                json_path = argv[++i];
            }
        } else if (std::strcmp(argv[i], "--csv") == 0) {
            write_csv = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                csv_compression_path = argv[++i];
            }
        }
    }

    config.enable_graphics = show_graphics;

    benchmark::BenchmarkOrchestrator orchestrator;
    auto report = orchestrator.run(config);

    if (!report.graphics_matrix_text.empty()) {
        std::cout << report.graphics_matrix_text << "\n\n";
    }
    if (!report.compression_matrix_text.empty()) {
        std::cout << report.compression_matrix_text << "\n\n";
    }
    std::cout << report.summary_text << "\n";

    if (write_json) {
        std::ofstream ofs(json_path);
        ofs << report.full_json;
        std::cout << "JSON written to " << json_path << "\n";
    }

    if (write_csv && !report.compression_csv.empty()) {
        std::ofstream ofs(csv_compression_path);
        ofs << report.compression_csv;
        std::cout << "Compression CSV written to " << csv_compression_path << "\n";
    }

    return 0;
}
