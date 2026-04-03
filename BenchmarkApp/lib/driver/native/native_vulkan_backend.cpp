#include "driver/native/native_vulkan_backend.h"

#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"

namespace benchmark {

GraphicsResult NativeVulkanBackend::run_compile_case(
    const std::string& workload_tier) {
    auto workload = workload_for_tier(workload_tier);
    auto profile = native_vulkan_profile();
    return simulate_graphics_timing(
        profile, workload, "Android", "Vulkan", "Native",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

// Self-registration
static const bool _reg_gfx_native_vulkan = (GraphicsBackendRegistry::instance().add(
    {"native_vulkan", "Android", "Vulkan", "Native", native_vulkan_profile}), true);

}  // namespace benchmark
