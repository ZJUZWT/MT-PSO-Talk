#include "driver/graphics_benchmark.h"

#include <memory>

#include "driver/common/graphics_backend.h"
#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"
#include "shaders/manifests/shader_workload.h"

namespace benchmark {

namespace {

std::vector<GraphicsResult> run_with_entry(
    const GraphicsBackendEntry& entry,
    const GraphicsBenchmarkConfig& config) {
    std::vector<GraphicsResult> results;
    auto profile = entry.make_profile();
    auto workloads = make_all_workloads();

    for (const auto& wl : workloads) {
        for (int iter = 0; iter < config.iterations_per_case; ++iter) {
            if (config.run_cold_cache) {
                results.push_back(simulate_graphics_timing(
                    profile, wl, entry.platform, entry.api, entry.driver_mode,
                    /*cold_cache=*/true, iter));
            }
            if (config.run_warm_cache) {
                results.push_back(simulate_graphics_timing(
                    profile, wl, entry.platform, entry.api, entry.driver_mode,
                    /*cold_cache=*/false, iter));
            }
        }
    }
    return results;
}

}  // namespace

std::vector<GraphicsResult> GraphicsBenchmark::run_all(
    const GraphicsBenchmarkConfig& config) {
    std::vector<GraphicsResult> all_results;
    auto& registry = GraphicsBackendRegistry::instance();
    for (const auto& entry : registry.all()) {
        auto results = run_with_entry(entry, config);
        all_results.insert(all_results.end(), results.begin(), results.end());
    }
    return all_results;
}

std::vector<GraphicsResult> GraphicsBenchmark::run_backend(
    const std::string& mode,
    const GraphicsBenchmarkConfig& config) {
    auto& registry = GraphicsBackendRegistry::instance();
    for (const auto& entry : registry.all()) {
        if (entry.mode == mode) {
            return run_with_entry(entry, config);
        }
    }
    return {};
}

}  // namespace benchmark
