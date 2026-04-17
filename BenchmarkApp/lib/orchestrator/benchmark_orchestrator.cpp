#include "orchestrator/benchmark_orchestrator.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <map>
#include <numeric>
#include <sstream>
#include <set>

#include "driver/common/device_info.h"
#include "driver/graphics_benchmark.h"
#include "compression/compression_benchmark.h"
#include "results/exporters/json_exporter.h"
#include "results/exporters/csv_exporter.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace benchmark {
namespace {

std::string detect_platform_name() {
#if defined(__ANDROID__)
    return "Android";
#elif defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__) && TARGET_OS_IPHONE
    return "iOS";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

DeviceInfo resolve_device_info(const OrchestratorConfig& config) {
    DeviceInfo info = query_device_info();

    if (!config.device_model.empty()) {
        info.device_model = config.device_model;
    }
    if (!config.soc.empty()) {
        info.soc = config.soc;
    }
    if (!config.os_version.empty()) {
        info.os_version = config.os_version;
    }

    if (info.device_model.empty()) {
        info.device_model = "Unknown Device";
    }
    if (info.soc.empty()) {
        info.soc = "Unknown SoC";
    }
    if (info.os_version.empty()) {
        info.os_version = detect_platform_name();
    }

    return info;
}

void apply_device_info(std::vector<GraphicsResult>& results, const DeviceInfo& info) {
    for (auto& result : results) {
        result.device_model = info.device_model;
        result.soc = info.soc;
        result.os_version = info.os_version;
    }
}

void apply_device_info(std::vector<CompressionResult>& results, const DeviceInfo& info) {
    for (auto& result : results) {
        result.device_model = info.device_model;
        result.soc = info.soc;
        result.os_version = info.os_version;
    }
}

template <typename T>
const T* first_result(const std::vector<T>& results) {
    return results.empty() ? nullptr : &results.front();
}

const CompressionResult* first_compression_result(const BenchmarkReport& report) {
    return first_result(report.compression_results);
}

const GraphicsResult* first_graphics_result(const BenchmarkReport& report) {
    return first_result(report.graphics_results);
}

}  // namespace

BenchmarkReport BenchmarkOrchestrator::run(const OrchestratorConfig& config) {
    BenchmarkReport report;
    const auto device_info = resolve_device_info(config);
    const auto platform_name = detect_platform_name();

    if (config.enable_graphics) {
        GraphicsBenchmarkConfig gcfg;
        gcfg.iterations_per_case = config.graphics_iterations;
        gcfg.run_cold_cache = config.graphics_cold_cache;
        gcfg.run_warm_cache = config.graphics_warm_cache;

        GraphicsBenchmark gb;
        report.graphics_results = gb.run_all(gcfg);
    }

    if (config.enable_compression) {
        CompressionBenchmarkConfig ccfg;
        ccfg.iterations = config.compression_iterations;
        ccfg.payload_sizes = config.compression_payload_sizes;

        CompressionBenchmark cb;
        report.compression_results = cb.run_all(platform_name, ccfg);
    }

    apply_device_info(report.graphics_results, device_info);
    apply_device_info(report.compression_results, device_info);

    report.graphics_matrix_text = format_graphics_matrix(report.graphics_results);
    report.compression_matrix_text = format_compression_matrix(report.compression_results);
    report.full_json = format_full_json(report);
    report.summary_text = format_summary(report);

    if (!report.graphics_results.empty()) {
        std::string csv = graphics_csv_header() + "\n";
        for (const auto& r : report.graphics_results) {
            csv += export_graphics_csv_row(r) + "\n";
        }
        report.graphics_csv = csv;
    }
    if (!report.compression_results.empty()) {
        std::string csv = compression_csv_header() + "\n";
        for (const auto& r : report.compression_results) {
            csv += export_compression_csv_row(r) + "\n";
        }
        report.compression_csv = csv;
    }

    return report;
}

// ──────────────────────────────────────────────────────────────────────────────
// Graphics Matrix
// ──────────────────────────────────────────────────────────────────────────────

std::string BenchmarkOrchestrator::format_graphics_matrix(
    const std::vector<GraphicsResult>& results) {

    if (results.empty()) return {};

    // Collect unique backends (driver_mode + " " + api) and case names, preserving order
    std::vector<std::string> backends_ordered;
    std::set<std::string> backends_seen;
    std::vector<std::string> cases_ordered;
    std::set<std::string> cases_seen;

    for (const auto& r : results) {
        std::string backend = r.platform + " " + r.driver_mode + " " + r.api;
        if (backends_seen.insert(backend).second) {
            backends_ordered.push_back(backend);
        }
        if (cases_seen.insert(r.case_name).second) {
            cases_ordered.push_back(r.case_name);
        }
    }

    // Average total_us by (backend, cold_cache, case_name)
    struct Key {
        std::string backend;
        bool cold_cache;
        std::string case_name;
        bool operator<(const Key& o) const {
            if (backend != o.backend) return backend < o.backend;
            if (cold_cache != o.cold_cache) return cold_cache < o.cold_cache;
            return case_name < o.case_name;
        }
    };

    std::map<Key, std::vector<int64_t>> grouped;
    for (const auto& r : results) {
        std::string backend = r.platform + " " + r.driver_mode + " " + r.api;
        grouped[{backend, r.cold_cache, r.case_name}].push_back(r.total_us);
    }

    auto avg = [](const std::vector<int64_t>& v) -> int64_t {
        if (v.empty()) return 0;
        return std::accumulate(v.begin(), v.end(), int64_t(0)) / static_cast<int64_t>(v.size());
    };

    // Build short tier labels from case names
    // e.g. "S1_BasePBR" -> "S1 BasePBR"
    auto short_label = [](const std::string& cn) -> std::string {
        std::string s = cn;
        for (auto& c : s) {
            if (c == '_') c = ' ';
        }
        return s;
    };

    // Calculate column widths
    const int backend_col = 22;
    const int cache_col = 6;
    const int tier_col = 16;

    std::ostringstream out;
    // Title
    out << "PSO Compilation Benchmark \xe2\x80\x94 Graphics Matrix (\xce\xbcs)\n";

    // Header separator
    auto full_width = [&]() -> int {
        return backend_col + 3 + cache_col + 3 +
               static_cast<int>(cases_ordered.size()) * (tier_col + 3);
    };
    std::string sep_heavy(full_width(), '\xe2');
    // Use simple chars for cross-platform compat
    auto heavy_line = [&]() {
        int w = full_width();
        for (int i = 0; i < w; ++i) out << "\xe2\x95\x90";
        out << "\n";
    };
    auto light_line = [&]() {
        int w = full_width();
        for (int i = 0; i < w; ++i) out << "\xe2\x94\x80";
        out << "\n";
    };

    // Header row
    heavy_line();
    out << " " << std::left << std::setw(backend_col) << "Backend"
        << " \xe2\x94\x82 " << std::left << std::setw(cache_col) << "Cache";

    for (const auto& cn : cases_ordered) {
        out << " \xe2\x94\x82 " << std::right << std::setw(tier_col) << short_label(cn);
    }
    out << " \xe2\x94\x82\n";
    heavy_line();

    // Data rows
    for (size_t bi = 0; bi < backends_ordered.size(); ++bi) {
        const auto& backend = backends_ordered[bi];

        // Cold row
        out << " " << std::left << std::setw(backend_col) << backend
            << " \xe2\x94\x82 " << std::left << std::setw(cache_col) << "Cold";
        for (const auto& cn : cases_ordered) {
            int64_t val = avg(grouped[{backend, true, cn}]);
            out << " \xe2\x94\x82 " << std::right << std::setw(tier_col) << val;
        }
        out << " \xe2\x94\x82\n";

        // Warm row
        out << " " << std::left << std::setw(backend_col) << ""
            << " \xe2\x94\x82 " << std::left << std::setw(cache_col) << "Warm";
        for (const auto& cn : cases_ordered) {
            int64_t val = avg(grouped[{backend, false, cn}]);
            out << " \xe2\x94\x82 " << std::right << std::setw(tier_col) << val;
        }
        out << " \xe2\x94\x82\n";

        // Separator
        if (bi + 1 < backends_ordered.size()) {
            light_line();
        }
    }

    heavy_line();

    return out.str();
}

// ──────────────────────────────────────────────────────────────────────────────
// Compression Matrix
// ──────────────────────────────────────────────────────────────────────────────

std::string BenchmarkOrchestrator::format_compression_matrix(
    const std::vector<CompressionResult>& results) {

    if (results.empty()) return {};

    // Use the first payload size found
    int64_t target_size = results.front().input_size;

    // Average by algorithm for the target payload size
    struct AlgoStats {
        std::string algorithm;
        int64_t input_size = 0;
        double compressed_size_avg = 0;
        double ratio_avg = 0;
        double compress_us_avg = 0;
        double decompress_us_avg = 0;
        double throughput_avg = 0;
        int count = 0;
    };

    // Preserve order of algorithms
    std::vector<std::string> algo_order;
    std::set<std::string> algo_seen;
    std::map<std::string, AlgoStats> stats;

    for (const auto& r : results) {
        if (r.input_size != target_size) continue;

        if (algo_seen.insert(r.algorithm).second) {
            algo_order.push_back(r.algorithm);
        }
        auto& s = stats[r.algorithm];
        s.algorithm = r.algorithm;
        s.input_size = r.input_size;
        s.compressed_size_avg += static_cast<double>(r.compressed_size);
        s.ratio_avg += r.compression_ratio;
        s.compress_us_avg += static_cast<double>(r.compress_us);
        s.decompress_us_avg += static_cast<double>(r.decompress_us);
        s.throughput_avg += r.throughput_mbps;
        s.count++;
    }

    for (auto& kv : stats) {
        auto& s = kv.second;
        if (s.count > 0) {
            s.compressed_size_avg /= s.count;
            s.ratio_avg /= s.count;
            s.compress_us_avg /= s.count;
            s.decompress_us_avg /= s.count;
            s.throughput_avg /= s.count;
        }
    }

    std::ostringstream out;
    int64_t payload_kb = target_size / 1024;
    out << "Compression Benchmark (" << payload_kb << "KB payload)\n";

    // Column widths
    const int algo_w = 12;
    const int comp_w = 12;
    const int ratio_w = 8;
    const int ctime_w = 14;
    const int dtime_w = 16;
    const int thru_w = 12;

    auto full_width = [&]() -> int {
        return algo_w + 3 + comp_w + 3 + ratio_w + 3 + ctime_w + 3 + dtime_w + 3 + thru_w + 3;
    };

    auto heavy_line = [&]() {
        int w = full_width();
        for (int i = 0; i < w; ++i) out << "\xe2\x95\x90";
        out << "\n";
    };

    // Header
    heavy_line();
    out << " " << std::left << std::setw(algo_w) << "Algorithm"
        << " \xe2\x94\x82 " << std::right << std::setw(comp_w) << "Compressed"
        << " \xe2\x94\x82 " << std::right << std::setw(ratio_w) << "Ratio"
        << " \xe2\x94\x82 " << std::right << std::setw(ctime_w) << "Compress(\xce\xbcs)"
        << " \xe2\x94\x82 " << std::right << std::setw(dtime_w) << "Decompress(\xce\xbcs)"
        << " \xe2\x94\x82 " << std::right << std::setw(thru_w) << "Throughput"
        << " \xe2\x94\x82\n";
    heavy_line();

    for (const auto& algo_name : algo_order) {
        const auto& s = stats.at(algo_name);

        int64_t comp_kb = static_cast<int64_t>(s.compressed_size_avg / 1024.0 + 0.5);
        std::string comp_str = std::to_string(comp_kb) + " KB";

        std::ostringstream ratio_ss;
        ratio_ss << std::fixed << std::setprecision(2) << s.ratio_avg << "x";

        int64_t compress_us = static_cast<int64_t>(s.compress_us_avg + 0.5);
        int64_t decompress_us = static_cast<int64_t>(s.decompress_us_avg + 0.5);

        std::ostringstream thru_ss;
        thru_ss << std::fixed << std::setprecision(0) << s.throughput_avg << " MB/s";

        out << " " << std::left << std::setw(algo_w) << s.algorithm
            << " \xe2\x94\x82 " << std::right << std::setw(comp_w) << comp_str
            << " \xe2\x94\x82 " << std::right << std::setw(ratio_w) << ratio_ss.str()
            << " \xe2\x94\x82 " << std::right << std::setw(ctime_w) << compress_us
            << " \xe2\x94\x82 " << std::right << std::setw(dtime_w) << decompress_us
            << " \xe2\x94\x82 " << std::right << std::setw(thru_w) << thru_ss.str()
            << " \xe2\x94\x82\n";
    }

    heavy_line();

    return out.str();
}

// ──────────────────────────────────────────────────────────────────────────────
// Summary
// ──────────────────────────────────────────────────────────────────────────────

std::string BenchmarkOrchestrator::format_summary(const BenchmarkReport& report) {
    std::ostringstream out;
    out << "Compression Benchmark Summary\n";
    out << "\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90"
           "\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90"
           "\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90"
           "\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90"
           "\xe2\x95\x90\n";

    std::string dev = "Unknown Device";
    std::string soc_str = "Unknown SoC";
    std::string os = "Unknown OS";
    if (const auto* graphics = first_graphics_result(report)) {
        dev = graphics->device_model;
        soc_str = graphics->soc;
        os = graphics->os_version;
    } else if (const auto* compression = first_compression_result(report)) {
        dev = compression->device_model;
        soc_str = compression->soc;
        os = compression->os_version;
    }
    out << "Platform: " << os << " | Device: " << dev << " | SoC: " << soc_str << "\n\n";

    if (!report.graphics_results.empty()) {
        std::set<std::string> backends, cases;
        bool has_cold = false, has_warm = false;
        for (const auto& r : report.graphics_results) {
            backends.insert(r.platform + " " + r.driver_mode + " " + r.api);
            cases.insert(r.case_name);
            if (r.cold_cache) has_cold = true; else has_warm = true;
        }
        int cache_modes = (has_cold ? 1 : 0) + (has_warm ? 1 : 0);

        out << "Graphics: " << backends.size() << " backends \xc3\x97 "
            << cases.size() << " workloads \xc3\x97 "
            << cache_modes << " cache modes = "
            << report.graphics_results.size() << " results\n";

        // Find fastest warm S1 and slowest cold S4
        int64_t fastest_warm = INT64_MAX;
        std::string fastest_name;
        int64_t slowest_cold = 0;
        std::string slowest_name;

        for (const auto& r : report.graphics_results) {
            std::string backend = r.platform + " " + r.driver_mode + " " + r.api;
            if (!r.cold_cache && r.total_us < fastest_warm) {
                fastest_warm = r.total_us;
                fastest_name = backend;
            }
            if (r.cold_cache && r.total_us > slowest_cold) {
                slowest_cold = r.total_us;
                slowest_name = backend;
            }
        }

        out << "  Fastest (warm): " << fastest_name << " \xe2\x80\x94 " << fastest_warm << " \xce\xbcs\n";
        out << "  Slowest (cold): " << slowest_name << " \xe2\x80\x94 " << slowest_cold << " \xce\xbcs\n";
        out << "\n";
    }

    if (!report.compression_results.empty()) {
        std::set<std::string> algos;
        std::set<int64_t> sizes;
        for (const auto& r : report.compression_results) {
            algos.insert(r.algorithm);
            sizes.insert(r.input_size);
        }

        out << "Compression: " << algos.size() << " algorithms \xc3\x97 "
            << sizes.size() << " payload size(s) = "
            << report.compression_results.size() << " results\n";

        double best_ratio = 0;
        std::string best_ratio_algo;
        int64_t fastest_decomp = INT64_MAX;
        std::string fastest_decomp_algo;

        for (const auto& r : report.compression_results) {
            if (r.status != "passed") {
                continue;
            }
            if (r.compression_ratio > best_ratio) {
                best_ratio = r.compression_ratio;
                best_ratio_algo = r.algorithm;
            }
            if (r.decompress_us < fastest_decomp) {
                fastest_decomp = r.decompress_us;
                fastest_decomp_algo = r.algorithm;
            }
        }

        std::ostringstream ratio_ss;
        ratio_ss << std::fixed << std::setprecision(2) << best_ratio << "x";
        out << "  Best ratio: " << best_ratio_algo << " \xe2\x80\x94 " << ratio_ss.str() << "\n";
        if (!fastest_decomp_algo.empty()) {
            out << "  Fastest decompress: " << fastest_decomp_algo
                << " \xe2\x80\x94 " << fastest_decomp << " \xce\xbcs\n";
        }
    }

    return out.str();
}

// ──────────────────────────────────────────────────────────────────────────────
// Full JSON
// ──────────────────────────────────────────────────────────────────────────────

std::string BenchmarkOrchestrator::format_full_json(const BenchmarkReport& report) {
    std::string dev = "Unknown Device";
    std::string soc_str = "Unknown SoC";
    std::string os = "Unknown OS";
    if (const auto* graphics = first_graphics_result(report)) {
        dev = graphics->device_model;
        soc_str = graphics->soc;
        os = graphics->os_version;
    } else if (const auto* compression = first_compression_result(report)) {
        dev = compression->device_model;
        soc_str = compression->soc;
        os = compression->os_version;
    }

    std::string json = "{";
    json += "\"benchmark\":\"Compression Benchmark\",";
    json += "\"device\":{";
    json += "\"model\":\"" + dev + "\",";
    json += "\"soc\":\"" + soc_str + "\",";
    json += "\"os\":\"" + os + "\"";
    json += "},";
    json += "\"graphics\":" + export_graphics_json_array(report.graphics_results) + ",";
    json += "\"compression\":" + export_compression_json_array(report.compression_results);
    json += "}";
    return json;
}

}  // namespace benchmark
