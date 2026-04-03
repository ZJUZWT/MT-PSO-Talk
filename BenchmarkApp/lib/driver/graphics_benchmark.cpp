#include "driver/graphics_benchmark.h"

#include <memory>

#include "driver/common/graphics_backend.h"
#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"
#include "driver/metal/metal_backend.h"
#include "driver/opengl/opengl_backend.h"
#include "driver/vulkan/vulkan_backend.h"
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

// Run a real GPU backend for all workloads
std::vector<GraphicsResult> run_real_backend(
    GraphicsBackend& backend,
    const GraphicsBenchmarkConfig& config) {
    std::vector<GraphicsResult> results;
    auto workloads = make_all_workloads();

    for (const auto& wl : workloads) {
        for (int iter = 0; iter < config.iterations_per_case; ++iter) {
            auto result = backend.run_compile_case(wl.tier);
            result.iteration_index = iter;
            result.cold_cache = (iter == 0);
            results.push_back(result);
        }
    }
    return results;
}

}  // namespace

std::vector<GraphicsResult> GraphicsBenchmark::run_all(
    const GraphicsBenchmarkConfig& config) {
    std::vector<GraphicsResult> all_results;

    if (config.use_real_backends) {
        // Try real Vulkan
        {
            RealVulkanBackend vk;
            auto results = run_real_backend(vk, config);
            all_results.insert(all_results.end(), results.begin(), results.end());
        }
        // Try real OpenGL
        {
            RealOpenGLBackend gl;
            auto results = run_real_backend(gl, config);
            all_results.insert(all_results.end(), results.begin(), results.end());
        }
        // Try real Metal
        {
            RealMetalBackend mtl;
            auto results = run_real_backend(mtl, config);
            all_results.insert(all_results.end(), results.begin(), results.end());
        }
        return all_results;
    }

    // Default: simulation via registry
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

    if (config.use_real_backends) {
        if (mode == "vulkan" || mode == "native_vulkan") {
            RealVulkanBackend vk;
            return run_real_backend(vk, config);
        }
        if (mode == "opengl" || mode == "native_opengl") {
            RealOpenGLBackend gl;
            return run_real_backend(gl, config);
        }
        if (mode == "metal") {
            RealMetalBackend mtl;
            return run_real_backend(mtl, config);
        }
    }

    // Fallback: simulation via registry
    auto& registry = GraphicsBackendRegistry::instance();
    for (const auto& entry : registry.all()) {
        if (entry.mode == mode) {
            return run_with_entry(entry, config);
        }
    }
    return {};
}

}  // namespace benchmark
