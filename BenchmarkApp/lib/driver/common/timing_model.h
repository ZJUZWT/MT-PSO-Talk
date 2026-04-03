#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "results/schema/result_types.h"
#include "shaders/manifests/shader_workload.h"

namespace benchmark {

struct TimingProfile {
    // Base latencies in microseconds for S1_BasePBR workload
    int64_t base_shader_compile_us;
    int64_t base_link_pipeline_us;
    int64_t base_first_draw_us;
    // Scaling factors for complexity
    double instruction_scale;     // per 100 instructions above baseline
    double permutation_scale;     // per permutation above baseline
    // Cold cache multiplier
    double cold_cache_multiplier;
    // Mesa stage percentages (empty for native)
    struct MesaStageProfile {
        std::string name;
        double percentage;  // fraction of total compile time
    };
    std::vector<MesaStageProfile> mesa_stages;
};

// Pre-built timing profiles
TimingProfile mesa_opengl_profile();
TimingProfile mesa_vulkan_profile();
TimingProfile native_opengl_profile();
TimingProfile native_vulkan_profile();
TimingProfile metal_profile();

// Lookup a ShaderWorkload by tier name
ShaderWorkload workload_for_tier(const std::string& tier);

// Generate a GraphicsResult from a timing profile and workload
GraphicsResult simulate_graphics_timing(
    const TimingProfile& profile,
    const ShaderWorkload& workload,
    const std::string& platform,
    const std::string& api,
    const std::string& driver_mode,
    bool cold_cache,
    int iteration_index);

}  // namespace benchmark
