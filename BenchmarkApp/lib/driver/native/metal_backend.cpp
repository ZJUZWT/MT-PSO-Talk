#include "driver/native/metal_backend.h"

#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"

namespace benchmark {

GraphicsResult MetalBackend::run_compile_case(
    const std::string& workload_tier) {
    auto workload = workload_for_tier(workload_tier);
    auto profile = metal_profile();
    return simulate_graphics_timing(
        profile, workload, "iOS", "Metal", "Native",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

// Self-registration
static const bool _reg_gfx_metal = (GraphicsBackendRegistry::instance().add(
    {"metal", "iOS", "Metal", "Native", metal_profile}), true);

}  // namespace benchmark
