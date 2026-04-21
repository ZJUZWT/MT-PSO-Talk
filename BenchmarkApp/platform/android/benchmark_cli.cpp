// Command-line compression benchmark runner for Android.
// Usage: ./pso_benchmark [--json output.json] [--csv output.csv] [--graphics]
//                        [--compression-iterations N]
//                        [--compression-warmup-iterations N]
//                        [--payload-sizes 64k,256k,1m]
//                        [--payload-profiles pso,high,low]

#include "orchestrator/benchmark_orchestrator.h"
#include <algorithm>
#include <climits>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

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
        std::string json_path, csv_path;
        bool show_graphics = false;
        benchmark::OrchestratorConfig config;

        for (int i = 1; i < argc; ++i) {
            if (std::string(argv[i]) == "--json" && i + 1 < argc) {
                json_path = argv[++i];
            } else if (std::string(argv[i]) == "--csv" && i + 1 < argc) {
                csv_path = argv[++i];
            } else if (std::string(argv[i]) == "--graphics") {
                show_graphics = true;
            } else if (std::string(argv[i]) == "--compression-iterations") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--compression-iterations requires a value");
                }
                config.compression_iterations =
                    parse_non_negative_int(argv[++i], "--compression-iterations");
            } else if (std::string(argv[i]) == "--compression-warmup-iterations") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--compression-warmup-iterations requires a value");
                }
                config.compression_warmup_iterations =
                    parse_non_negative_int(argv[++i], "--compression-warmup-iterations");
            } else if (std::string(argv[i]) == "--payload-sizes") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--payload-sizes requires a value");
                }
                config.compression_payload_sizes = parse_payload_sizes(argv[++i]);
            } else if (std::string(argv[i]) == "--payload-profiles") {
                if (i + 1 >= argc) {
                    throw std::invalid_argument("--payload-profiles requires a value");
                }
                config.compression_payload_profiles = parse_payload_profiles(argv[++i]);
            }
        }

        config.enable_graphics = show_graphics;
        config.graphics_iterations = 3;

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
    } catch (const std::exception& error) {
        std::cerr << "pso_benchmark error: " << error.what() << "\n";
        return 1;
    }
}
