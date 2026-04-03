#include "driver/common/timing_model.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <string>

namespace benchmark {

TimingProfile mesa_opengl_profile() {
    TimingProfile p{};
    p.base_shader_compile_us = 8000;
    p.base_link_pipeline_us = 12000;
    p.base_first_draw_us = 3000;
    p.instruction_scale = 0.15;
    p.permutation_scale = 0.08;
    p.cold_cache_multiplier = 4.0;
    p.mesa_stages = {
        {"frontend_ingest", 0.12},
        {"ir_conversion", 0.18},
        {"optimization", 0.35},
        {"backend_codegen", 0.25},
        {"link_finalization", 0.10},
    };
    return p;
}

TimingProfile mesa_vulkan_profile() {
    TimingProfile p{};
    p.base_shader_compile_us = 6000;
    p.base_link_pipeline_us = 15000;
    p.base_first_draw_us = 2000;
    p.instruction_scale = 0.12;
    p.permutation_scale = 0.06;
    p.cold_cache_multiplier = 3.5;
    p.mesa_stages = {
        {"spirv_parse", 0.08},
        {"nir_lowering", 0.20},
        {"optimization", 0.30},
        {"backend_codegen", 0.28},
        {"pipeline_finalization", 0.14},
    };
    return p;
}

TimingProfile native_opengl_profile() {
    TimingProfile p{};
    p.base_shader_compile_us = 5000;
    p.base_link_pipeline_us = 8000;
    p.base_first_draw_us = 2500;
    p.instruction_scale = 0.10;
    p.permutation_scale = 0.05;
    p.cold_cache_multiplier = 3.0;
    return p;
}

TimingProfile native_vulkan_profile() {
    TimingProfile p{};
    p.base_shader_compile_us = 3000;
    p.base_link_pipeline_us = 10000;
    p.base_first_draw_us = 1500;
    p.instruction_scale = 0.08;
    p.permutation_scale = 0.04;
    p.cold_cache_multiplier = 2.5;
    return p;
}

TimingProfile metal_profile() {
    TimingProfile p{};
    p.base_shader_compile_us = 2000;
    p.base_link_pipeline_us = 5000;
    p.base_first_draw_us = 1000;
    p.instruction_scale = 0.06;
    p.permutation_scale = 0.03;
    p.cold_cache_multiplier = 2.0;
    return p;
}

ShaderWorkload workload_for_tier(const std::string& tier) {
    if (tier == "S1_BasePBR") return make_base_pbr_workload();
    if (tier == "S2_MaterialLayered") return make_material_layered_workload();
    if (tier == "S3_FeatureHeavy") return make_feature_heavy_workload();
    if (tier == "S4_PermutationStress") return make_permutation_stress_workload();
    // Fallback to base PBR for unknown tiers
    return make_base_pbr_workload();
}

static uint32_t simple_hash(const std::string& s) {
    uint32_t h = 0;
    for (char c : s) {
        h = h * 31 + static_cast<uint32_t>(c);
    }
    return h;
}

GraphicsResult simulate_graphics_timing(
    const TimingProfile& profile,
    const ShaderWorkload& workload,
    const std::string& platform,
    const std::string& api,
    const std::string& driver_mode,
    bool cold_cache,
    int iteration_index) {

    // 1. Calculate complexity factor
    double complexity_factor = 1.0
        + (workload.instruction_count - 200) / 100.0 * profile.instruction_scale
        + (workload.permutation_count - 8) * profile.permutation_scale;

    // 2. Scale base times
    double shader_compile = profile.base_shader_compile_us * complexity_factor;
    double link_pipeline = profile.base_link_pipeline_us * complexity_factor;
    double first_draw = profile.base_first_draw_us * complexity_factor;

    // 3. Cold cache multiplier
    if (cold_cache) {
        shader_compile *= profile.cold_cache_multiplier;
        link_pipeline *= profile.cold_cache_multiplier;
        first_draw *= profile.cold_cache_multiplier;
    }

    // 4. Deterministic jitter ±10%
    uint32_t seed = simple_hash(workload.tier + std::to_string(iteration_index));
    double jitter = static_cast<double>(seed % 200) - 100.0;
    jitter /= 1000.0;  // ±0.1

    shader_compile *= (1.0 + jitter);
    // Use different jitter for each phase
    uint32_t seed2 = simple_hash(workload.tier + std::to_string(iteration_index) + "link");
    double jitter2 = (static_cast<double>(seed2 % 200) - 100.0) / 1000.0;
    link_pipeline *= (1.0 + jitter2);

    uint32_t seed3 = simple_hash(workload.tier + std::to_string(iteration_index) + "draw");
    double jitter3 = (static_cast<double>(seed3 % 200) - 100.0) / 1000.0;
    first_draw *= (1.0 + jitter3);

    // Build result
    GraphicsResult result{};
    result.platform = platform;
    result.api = api;
    result.driver_mode = driver_mode;
    result.case_name = workload.tier;
    result.iteration_index = iteration_index;
    result.cold_cache = cold_cache;
    result.status = "passed";
    result.shader_compile_us = static_cast<int64_t>(std::round(shader_compile));
    result.link_pipeline_create_us = static_cast<int64_t>(std::round(link_pipeline));
    result.first_draw_ready_us = static_cast<int64_t>(std::round(first_draw));
    result.total_us = result.shader_compile_us + result.link_pipeline_create_us
                      + result.first_draw_ready_us;

    // 5. Mesa stage breakdowns
    for (const auto& stage : profile.mesa_stages) {
        StageTiming st;
        st.name = stage.name;
        st.duration_us = static_cast<int64_t>(
            std::round(shader_compile * stage.percentage));
        result.stage_breakdown.push_back(st);
    }

    return result;
}

}  // namespace benchmark
