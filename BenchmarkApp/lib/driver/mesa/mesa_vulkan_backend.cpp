#include "driver/mesa/mesa_vulkan_backend.h"

#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"

namespace benchmark {

GraphicsResult MesaVulkanBackend::run_compile_case(
    const std::string& workload_tier) {
    auto workload = workload_for_tier(workload_tier);
    auto profile = mesa_vulkan_profile();
    return simulate_graphics_timing(
        profile, workload, "Android", "Vulkan", "Mesa",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

// Self-registration
static const bool _reg_gfx_mesa_vulkan = (GraphicsBackendRegistry::instance().add(
    {"mesa_vulkan", "Android", "Vulkan", "Mesa", mesa_vulkan_profile}), true);

}  // namespace benchmark
