#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

#include "driver/common/timing_model.h"
#include "shaders/manifests/shader_workload.h"
#include "test_support/assert.h"

using namespace benchmark;

int main() {
    // test_workload_for_tier
    {
        auto w1 = workload_for_tier("S1_BasePBR");
        test_support::expect_equal(w1.tier, std::string("S1_BasePBR"), "w1 tier");
        test_support::expect_equal(w1.instruction_count, 200, "w1 instructions");

        auto w3 = workload_for_tier("S3_FeatureHeavy");
        test_support::expect_equal(w3.tier, std::string("S3_FeatureHeavy"), "w3 tier");
        test_support::expect_equal(w3.instruction_count, 800, "w3 instructions");
    }

    // test_profiles_produce_positive_values + stage duration sum check
    {
        auto workload = make_base_pbr_workload();
        auto profiles = {
            mesa_opengl_profile(), mesa_vulkan_profile(),
            native_opengl_profile(), native_vulkan_profile(),
            metal_profile()
        };
        for (const auto& profile : profiles) {
            auto result = simulate_graphics_timing(
                profile, workload, "Test", "TestAPI", "TestDriver",
                false, 0);
            test_support::expect_greater_than(result.shader_compile_us, static_cast<int64_t>(0),
                "shader_compile_us > 0");
            test_support::expect_greater_than(result.link_pipeline_create_us, static_cast<int64_t>(0),
                "link_pipeline_create_us > 0");
            test_support::expect_greater_than(result.first_draw_ready_us, static_cast<int64_t>(0),
                "first_draw_ready_us > 0");
            test_support::expect_equal(result.total_us,
                result.shader_compile_us + result.link_pipeline_create_us + result.first_draw_ready_us,
                "total_us == sum of components");
            test_support::expect_equal(result.status, std::string("passed"), "status passed");

            // Verify stage durations sum to approximately shader_compile_us (within 5%)
            if (!result.stage_breakdown.empty()) {
                int64_t stage_sum = 0;
                for (const auto& s : result.stage_breakdown) {
                    stage_sum += s.duration_us;
                }
                double diff = std::abs(static_cast<double>(stage_sum - result.shader_compile_us));
                double tolerance = result.shader_compile_us * 0.05;
                test_support::expect_true(diff <= tolerance + 1,  // +1 for rounding
                    "stage durations sum ~= shader_compile_us (sum=" +
                    std::to_string(stage_sum) + " vs " +
                    std::to_string(result.shader_compile_us) + ")");
            }
        }
    }

    // test_cold_cache_slower_than_warm — verify cold > warm * 1.5
    {
        auto workload = make_base_pbr_workload();
        auto profile = mesa_opengl_profile();

        auto cold = simulate_graphics_timing(
            profile, workload, "Test", "GL", "Mesa", true, 0);
        auto warm = simulate_graphics_timing(
            profile, workload, "Test", "GL", "Mesa", false, 0);

        test_support::expect_greater_than(cold.total_us, warm.total_us, "cold > warm");
        double ratio = static_cast<double>(cold.total_us) / warm.total_us;
        test_support::expect_greater_than_double(ratio, 1.5,
            "cold/warm ratio > 1.5 (ratio=" + std::to_string(ratio) + ")");
        test_support::expect_true(ratio < 6.0,
            "cold/warm ratio < 6.0 (ratio=" + std::to_string(ratio) + ")");
    }

    // test_mesa_has_stage_breakdown
    {
        auto workload = make_base_pbr_workload();

        auto r1 = simulate_graphics_timing(
            mesa_opengl_profile(), workload, "T", "GL", "Mesa", false, 0);
        test_support::expect_true(!r1.stage_breakdown.empty(), "mesa_gl has stages");
        test_support::expect_equal(r1.stage_breakdown.size(), static_cast<size_t>(5),
            "mesa_gl 5 stages");

        auto r2 = simulate_graphics_timing(
            mesa_vulkan_profile(), workload, "T", "VK", "Mesa", false, 0);
        test_support::expect_true(!r2.stage_breakdown.empty(), "mesa_vk has stages");
        test_support::expect_equal(r2.stage_breakdown.size(), static_cast<size_t>(5),
            "mesa_vk 5 stages");

        auto r3 = simulate_graphics_timing(
            native_opengl_profile(), workload, "T", "GL", "Native", false, 0);
        test_support::expect_true(r3.stage_breakdown.empty(), "native_gl no stages");

        auto r4 = simulate_graphics_timing(
            native_vulkan_profile(), workload, "T", "VK", "Native", false, 0);
        test_support::expect_true(r4.stage_breakdown.empty(), "native_vk no stages");

        auto r5 = simulate_graphics_timing(
            metal_profile(), workload, "T", "Metal", "Native", false, 0);
        test_support::expect_true(r5.stage_breakdown.empty(), "metal no stages");
    }

    // test_backend_ordering: Metal < Native Vulkan < Native OpenGL < Mesa Vulkan < Mesa OpenGL
    {
        auto workload = make_feature_heavy_workload();

        auto metal_r = simulate_graphics_timing(
            metal_profile(), workload, "iOS", "Metal", "Native", false, 0);
        auto nat_vk_r = simulate_graphics_timing(
            native_vulkan_profile(), workload, "Android", "VK", "Native", false, 0);
        auto nat_gl_r = simulate_graphics_timing(
            native_opengl_profile(), workload, "Android", "GL", "Native", false, 0);
        auto mesa_vk_r = simulate_graphics_timing(
            mesa_vulkan_profile(), workload, "Android", "VK", "Mesa", false, 0);
        auto mesa_gl_r = simulate_graphics_timing(
            mesa_opengl_profile(), workload, "Android", "GL", "Mesa", false, 0);

        test_support::expect_less_than(metal_r.total_us, nat_vk_r.total_us,
            "Metal < Native Vulkan");
        test_support::expect_less_than(nat_vk_r.total_us, nat_gl_r.total_us,
            "Native Vulkan < Native OpenGL");
        test_support::expect_less_than(nat_gl_r.total_us, mesa_vk_r.total_us,
            "Native OpenGL < Mesa Vulkan");
        test_support::expect_less_than(mesa_vk_r.total_us, mesa_gl_r.total_us,
            "Mesa Vulkan < Mesa OpenGL");
    }

    // test_complexity_scaling: S4 > S1 * 1.5
    {
        auto profile = mesa_opengl_profile();
        auto s1 = make_base_pbr_workload();
        auto s4 = make_permutation_stress_workload();

        auto r1 = simulate_graphics_timing(
            profile, s1, "T", "GL", "Mesa", false, 0);
        auto r4 = simulate_graphics_timing(
            profile, s4, "T", "GL", "Mesa", false, 0);

        test_support::expect_greater_than(r4.total_us, r1.total_us,
            "S4 > S1");
        double scaling = static_cast<double>(r4.total_us) / r1.total_us;
        test_support::expect_greater_than_double(scaling, 1.5,
            "S4 > S1 * 1.5 (scaling=" + std::to_string(scaling) + ")");
    }

    // test_jitter: same workload different iteration gives different results
    {
        auto profile = metal_profile();
        auto workload = make_base_pbr_workload();

        auto r1 = simulate_graphics_timing(
            profile, workload, "T", "M", "N", false, 0);
        auto r2 = simulate_graphics_timing(
            profile, workload, "T", "M", "N", false, 0);

        // Same inputs -> same outputs (deterministic)
        test_support::expect_equal(r1.total_us, r2.total_us, "deterministic same iteration");
        test_support::expect_equal(r1.shader_compile_us, r2.shader_compile_us,
            "deterministic shader_compile");

        // Different iteration -> different values (jitter)
        auto r3 = simulate_graphics_timing(
            profile, workload, "T", "M", "N", false, 1);
        test_support::expect_true(r1.total_us != r3.total_us,
            "different iteration produces different total_us");
    }

    std::cout << "timing_model_test: all checks completed\n";
    return test_support::finish();
}
