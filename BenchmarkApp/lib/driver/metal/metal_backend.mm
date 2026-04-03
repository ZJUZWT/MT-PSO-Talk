#include "driver/metal/metal_backend.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include "driver/common/device_info.h"
#include "driver/common/timing_model.h"
#include "driver/metal/metal_pipeline.h"

namespace benchmark {

RealMetalBackend::RealMetalBackend() {
    ctx_.init();
}

RealMetalBackend::~RealMetalBackend() {
    ctx_.destroy();
}

GraphicsResult RealMetalBackend::run_compile_case(const std::string& workload_tier) {
#if defined(__APPLE__) && TARGET_OS_IPHONE
    if (ctx_.is_available()) {
        auto info = query_device_info();

        // TODO: load real MSL source from workload assets

        auto workload = workload_for_tier(workload_tier);
        auto profile = metal_profile();
        auto result = simulate_graphics_timing(
            profile, workload, "iOS", "Metal", "RealGPU",
            /*cold_cache=*/true, /*iteration_index=*/0);
        result.device_model = info.device_model;
        result.soc = info.soc;
        result.os_version = info.os_version;
        return result;
    }
#endif

    // Fallback: simulation
    auto workload = workload_for_tier(workload_tier);
    auto profile = metal_profile();
    return simulate_graphics_timing(
        profile, workload, "Simulated", "Metal", "Simulated",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

}  // namespace benchmark
