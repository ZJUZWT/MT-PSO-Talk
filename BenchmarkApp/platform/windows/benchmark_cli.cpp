#include <algorithm>
#include <climits>
#include <cstring>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "orchestrator/benchmark_orchestrator.h"

namespace {

std::string normalize_token(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), value.end());
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

int parse_non_negative_int(const char* value, const char* flag_name) {
    long long parsed = std::stoll(value);
    if (parsed < 0 || parsed > INT_MAX) {
        throw std::invalid_argument(std::string(flag_name) + " expects a non-negative integer");
    }
    return static_cast<int>(parsed);
}

int64_t parse_size_token(const std::string& raw_token) {
    std::string token = normalize_token(raw_token);
    int64_t multiplier = 1;

    if (token.size() >= 2 && token.substr(token.size() - 2) == "kb") {
        multiplier = 1024;
        token.erase(token.size() - 2);
    } else if (!token.empty() && token.back() == 'k') {
        multiplier = 1024;
        token.pop_back();
    } else if (token.size() >= 2 && token.substr(token.size() - 2) == "mb") {
        multiplier = 1024 * 1024;
        token.erase(token.size() - 2);
    } else if (!token.empty() && token.back() == 'm') {
        multiplier = 1024 * 1024;
        token.pop_back();
    } else if (!token.empty() && token.back() == 'b') {
        token.pop_back();
    }

    if (token.empty()) {
        throw std::invalid_argument("payload size token is empty");
    }

    return std::stoll(token) * multiplier;
}

std::string canonicalize_payload_profile(const std::string& raw_token) {
    std::string token = normalize_token(raw_token);
    if (token == "pso" || token == "pso_like") {
        return "pso_like";
    }
    if (token == "high" || token == "high_compressibility" || token == "compressible") {
        return "high_compressibility";
    }
    if (token == "low" || token == "low_compressibility" || token == "incompressible") {
        return "low_compressibility";
    }

    throw std::invalid_argument("unsupported payload profile: " + raw_token);
}

std::vector<int64_t> parse_payload_sizes(const char* value) {
    std::stringstream stream(value);
    std::string token;
    std::vector<int64_t> sizes;
    while (std::getline(stream, token, ',')) {
        if (!normalize_token(token).empty()) {
            sizes.push_back(parse_size_token(token));
        }
    }
    if (sizes.empty()) {
        throw std::invalid_argument("--payload-sizes requires at least one size");
    }
    return sizes;
}

std::vector<std::string> parse_payload_profiles(const char* value) {
    std::stringstream stream(value);
    std::string token;
    std::vector<std::string> profiles;
    while (std::getline(stream, token, ',')) {
        if (!normalize_token(token).empty()) {
            profiles.push_back(canonicalize_payload_profile(token));
        }
    }
    if (profiles.empty()) {
        throw std::invalid_argument("--payload-profiles requires at least one profile");
    }
    return profiles;
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        benchmark::OrchestratorConfig config;
        bool write_json = false;
        bool write_csv = false;
        std::string json_path = "benchmark_report.json";
        std::string csv_path = "compression_results.csv";

        for (int i = 1; i < argc; ++i) {
            if (std::strcmp(argv[i], "--graphics") == 0) {
                config.enable_graphics = true;
            } else if (std::strcmp(argv[i], "--json") == 0) {
                write_json = true;
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    json_path = argv[++i];
                }
            } else if (std::strcmp(argv[i], "--csv") == 0) {
                write_csv = true;
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    csv_path = argv[++i];
                }
            } else if (std::strcmp(argv[i], "--compression-iterations") == 0) {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--compression-iterations requires a value");
                }
                config.compression_iterations =
                    parse_non_negative_int(argv[++i], "--compression-iterations");
            } else if (std::strcmp(argv[i], "--compression-warmup-iterations") == 0) {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--compression-warmup-iterations requires a value");
                }
                config.compression_warmup_iterations =
                    parse_non_negative_int(argv[++i], "--compression-warmup-iterations");
            } else if (std::strcmp(argv[i], "--payload-sizes") == 0) {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--payload-sizes requires a value");
                }
                config.compression_payload_sizes = parse_payload_sizes(argv[++i]);
            } else if (std::strcmp(argv[i], "--payload-profiles") == 0) {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--payload-profiles requires a value");
                }
                config.compression_payload_profiles = parse_payload_profiles(argv[++i]);
            }
        }

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
            std::ofstream output(json_path);
            output << report.full_json;
        }

        if (write_csv && !report.compression_csv.empty()) {
            std::ofstream output(csv_path);
            output << report.compression_csv;
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "compression_benchmark_cli error: " << error.what() << "\n";
        return 1;
    }
}
