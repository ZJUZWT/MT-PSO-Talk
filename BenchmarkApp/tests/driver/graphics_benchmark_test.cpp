#include <cassert>
#include <cstdio>
#include <set>
#include <string>

#include "driver/graphics_benchmark.h"
#include "driver/common/graphics_backend.h"

using namespace benchmark;

static int tests_passed = 0;

static void test_run_all_returns_correct_count() {
    GraphicsBenchmark bench;
    GraphicsBenchmarkConfig config;
    config.iterations_per_case = 2;
    config.run_cold_cache = true;
    config.run_warm_cache = true;

    auto results = bench.run_all(config);

    // 5 backends * 4 workloads * 2 iterations * 2 (cold+warm) = 80
    assert(results.size() == 80);

    ++tests_passed;
    std::printf("  PASS: run_all returns correct count\n");
}

static void test_run_all_covers_all_backends() {
    GraphicsBenchmark bench;
    GraphicsBenchmarkConfig config;
    config.iterations_per_case = 1;
    config.run_cold_cache = true;
    config.run_warm_cache = false;

    auto results = bench.run_all(config);

    std::set<std::string> drivers;
    for (const auto& r : results) {
        drivers.insert(r.api + "_" + r.driver_mode);
    }
    assert(drivers.count("OpenGL_Mesa") == 1);
    assert(drivers.count("Vulkan_Mesa") == 1);
    assert(drivers.count("OpenGL_Native") == 1);
    assert(drivers.count("Vulkan_Native") == 1);
    assert(drivers.count("Metal_Native") == 1);

    ++tests_passed;
    std::printf("  PASS: run_all covers all 5 backends\n");
}

static void test_run_all_covers_all_workloads() {
    GraphicsBenchmark bench;
    GraphicsBenchmarkConfig config;
    config.iterations_per_case = 1;
    config.run_cold_cache = true;
    config.run_warm_cache = false;

    auto results = bench.run_all(config);

    std::set<std::string> tiers;
    for (const auto& r : results) {
        tiers.insert(r.case_name);
    }
    assert(tiers.count("S1_BasePBR") == 1);
    assert(tiers.count("S2_MaterialLayered") == 1);
    assert(tiers.count("S3_FeatureHeavy") == 1);
    assert(tiers.count("S4_PermutationStress") == 1);

    ++tests_passed;
    std::printf("  PASS: run_all covers all 4 workload tiers\n");
}

static void test_run_backend_specific() {
    GraphicsBenchmark bench;
    GraphicsBenchmarkConfig config;
    config.iterations_per_case = 1;
    config.run_cold_cache = true;
    config.run_warm_cache = true;

    auto results = bench.run_backend("metal", config);

    // 4 workloads * 1 iteration * 2 (cold+warm) = 8
    assert(results.size() == 8);
    for (const auto& r : results) {
        assert(r.api == "Metal");
        assert(r.driver_mode == "Native");
        assert(r.platform == "iOS");
    }

    ++tests_passed;
    std::printf("  PASS: run_backend(metal) returns correct results\n");
}

static void test_run_backend_unknown_returns_empty() {
    GraphicsBenchmark bench;
    auto results = bench.run_backend("unknown_backend");
    assert(results.empty());

    ++tests_passed;
    std::printf("  PASS: run_backend(unknown) returns empty\n");
}

static void test_cold_and_warm_flags() {
    GraphicsBenchmark bench;
    GraphicsBenchmarkConfig config;
    config.iterations_per_case = 1;

    // cold only
    config.run_cold_cache = true;
    config.run_warm_cache = false;
    auto cold_results = bench.run_backend("metal", config);
    for (const auto& r : cold_results) {
        assert(r.cold_cache == true);
    }

    // warm only
    config.run_cold_cache = false;
    config.run_warm_cache = true;
    auto warm_results = bench.run_backend("metal", config);
    for (const auto& r : warm_results) {
        assert(r.cold_cache == false);
    }

    ++tests_passed;
    std::printf("  PASS: cold/warm cache flags respected\n");
}

static void test_make_backend_factory() {
    auto mesa_gl = make_backend("mesa_opengl");
    assert(mesa_gl != nullptr);

    auto mesa_vk = make_backend("mesa_vulkan");
    assert(mesa_vk != nullptr);

    auto nat_gl = make_backend("native_opengl");
    assert(nat_gl != nullptr);

    auto nat_vk = make_backend("native_vulkan");
    assert(nat_vk != nullptr);

    auto metal = make_backend("metal");
    assert(metal != nullptr);

    auto unknown = make_backend("bogus");
    assert(unknown == nullptr);

    // Test that backends produce results
    auto r = mesa_gl->run_compile_case("S1_BasePBR");
    assert(r.total_us > 0);
    assert(r.status == "passed");
    assert(!r.stage_breakdown.empty());

    ++tests_passed;
    std::printf("  PASS: make_backend factory\n");
}

int main() {
    std::printf("Running graphics_benchmark tests...\n");
    test_run_all_returns_correct_count();
    test_run_all_covers_all_backends();
    test_run_all_covers_all_workloads();
    test_run_backend_specific();
    test_run_backend_unknown_returns_empty();
    test_cold_and_warm_flags();
    test_make_backend_factory();
    std::printf("All %d graphics_benchmark tests passed.\n", tests_passed);
    return 0;
}
