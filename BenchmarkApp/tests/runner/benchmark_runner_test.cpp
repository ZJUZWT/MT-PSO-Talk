#include "runner/benchmark_runner.h"
#include "test_support/assert.h"

#include <set>
#include <string>

int main() {
    benchmark::BenchmarkRunner runner;

    // Test all 8 modes individually
    const auto modes = benchmark::BenchmarkRunner::all_modes();
    test_support::expect_equal(modes.size(), static_cast<size_t>(8), "8 total modes");

    // 5 graphics modes
    std::string graphics_modes[] = {
        "android_mesa_vulkan", "android_mesa_opengl",
        "android_native_vulkan", "android_native_opengl",
        "ios_native_metal"
    };
    for (const auto& mode : graphics_modes) {
        const auto plan = runner.plan_for_mode(mode);
        test_support::expect_true(!plan.empty(), mode + " has cases");
        test_support::expect_equal(plan.size(), static_cast<size_t>(4), mode + " has 4 tiers");
        test_support::expect_true(!plan[0].is_compression, mode + " is not compression");

        // Verify workload_tier values for all graphics modes
        test_support::expect_equal(plan[0].workload_tier, std::string("S1_BasePBR"),
            mode + " tier 0");
        test_support::expect_equal(plan[1].workload_tier, std::string("S2_MaterialLayered"),
            mode + " tier 1");
        test_support::expect_equal(plan[2].workload_tier, std::string("S3_FeatureHeavy"),
            mode + " tier 2");
        test_support::expect_equal(plan[3].workload_tier, std::string("S4_PermutationStress"),
            mode + " tier 3");
    }

    // Specific driver_mode check
    const auto vulkan_plan = runner.plan_for_mode("android_mesa_vulkan");
    test_support::expect_equal(vulkan_plan.front().driver_mode, std::string("Mesa"),
        "mesa vulkan driver mode");

    // 3 compression modes
    std::string compression_modes[] = {
        "android_compression", "ios_compression", "windows_compression"
    };
    for (const auto& mode : compression_modes) {
        const auto plan = runner.plan_for_mode(mode);
        test_support::expect_equal(plan.size(), static_cast<size_t>(4),
            mode + " has 4 algorithms");
        for (const auto& c : plan) {
            test_support::expect_true(c.is_compression, mode + " is_compression == true");
            test_support::expect_true(!c.compression_algorithm.empty(),
                mode + " has compression_algorithm");
        }
    }

    // Verify specific compression algorithm order
    const auto android_comp = runner.plan_for_mode("android_compression");
    test_support::expect_equal(android_comp[0].compression_algorithm, std::string("zstd"),
        "zstd first");
    test_support::expect_equal(android_comp[1].compression_algorithm, std::string("lz4"),
        "lz4 second");
    test_support::expect_equal(android_comp[2].compression_algorithm, std::string("zlib"),
        "zlib third");
    test_support::expect_equal(android_comp[3].compression_algorithm, std::string("snappy"),
        "snappy fourth");

    // plan_all returns correct total count
    const auto all = runner.plan_all();
    // 5 graphics modes * 4 tiers + 3 compression modes * 4 algorithms = 20 + 12 = 32
    test_support::expect_equal(all.size(), static_cast<size_t>(32), "plan_all returns 32 cases");

    // Verify plan_all has both compression and graphics
    int compression_count = 0;
    int graphics_count = 0;
    for (const auto& c : all) {
        if (c.is_compression) ++compression_count;
        else ++graphics_count;
    }
    test_support::expect_equal(compression_count, 12, "plan_all 12 compression cases");
    test_support::expect_equal(graphics_count, 20, "plan_all 20 graphics cases");

    // Unknown mode returns empty
    const auto unknown = runner.plan_for_mode("unknown_mode");
    test_support::expect_true(unknown.empty(), "unknown mode is empty");

    const auto unknown2 = runner.plan_for_mode("");
    test_support::expect_true(unknown2.empty(), "empty mode is empty");

    return test_support::finish();
}
