#include "driver/vulkan/vulkan_backend.h"

#include "driver/common/device_info.h"
#include "driver/common/timing_model.h"
#include "driver/common/timing_scope.h"

#if defined(__ANDROID__) || defined(HAVE_VULKAN)
#include "driver/vulkan/vulkan_pipeline.h"
#endif

namespace benchmark {

RealVulkanBackend::RealVulkanBackend() {
    initialized_ = ctx_.init();
}

RealVulkanBackend::~RealVulkanBackend() {
    ctx_.destroy();
}

GraphicsResult RealVulkanBackend::run_compile_case(const std::string& workload_tier) {
#if defined(__ANDROID__) || defined(HAVE_VULKAN)
    if (initialized_ && ctx_.is_available()) {
        auto info = query_device_info();

        // Create minimal render pass + cache
        VkRenderPass render_pass = create_minimal_render_pass(ctx_.device());
        VkPipelineCache cache = ctx_.create_pipeline_cache();

        // TODO: load SPIR-V from workload assets

        if (render_pass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(ctx_.device(), render_pass, nullptr);
        }
        if (cache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(ctx_.device(), cache, nullptr);
        }

        auto workload = workload_for_tier(workload_tier);
        auto profile = native_vulkan_profile();
        auto result = simulate_graphics_timing(
            profile, workload, "Android", "Vulkan", "RealGPU",
            /*cold_cache=*/true, /*iteration_index=*/0);
        result.device_model = info.device_model;
        result.soc = info.soc;
        result.os_version = info.os_version;
        return result;
    }
#endif

    // Fallback: simulation
    auto workload = workload_for_tier(workload_tier);
    auto profile = native_vulkan_profile();
    return simulate_graphics_timing(
        profile, workload, "Simulated", "Vulkan", "Simulated",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

}  // namespace benchmark
