#include "driver/common/graphics_backend.h"
#include "test_support/assert.h"

#include <iostream>
#include <memory>
#include <string>

int main() {
    // Test all 5 backends via make_backend() factory
    struct BackendSpec {
        std::string mode;
        std::string expected_api;
        std::string expected_driver_mode;
        bool expect_stage_breakdown;  // Mesa backends have it, native don't
    };

    BackendSpec specs[] = {
        {"mesa_opengl",    "OpenGL",  "Mesa",   true},
        {"mesa_vulkan",    "Vulkan",  "Mesa",   true},
        {"native_opengl",  "OpenGL",  "Native", false},
        {"native_vulkan",  "Vulkan",  "Native", false},
        {"metal",          "Metal",   "Native", false},
    };

    for (const auto& spec : specs) {
        auto backend = benchmark::make_backend(spec.mode);
        test_support::expect_true(backend != nullptr, spec.mode + " backend not null");

        auto result = backend->run_compile_case("S1_BasePBR");

        test_support::expect_greater_than(result.total_us, static_cast<int64_t>(0),
            spec.mode + " total_us > 0");
        test_support::expect_greater_than(result.shader_compile_us, static_cast<int64_t>(0),
            spec.mode + " shader_compile_us > 0");
        test_support::expect_greater_than(result.link_pipeline_create_us, static_cast<int64_t>(0),
            spec.mode + " link_pipeline_create_us > 0");
        test_support::expect_equal(result.api, spec.expected_api,
            spec.mode + " api");
        test_support::expect_equal(result.driver_mode, spec.expected_driver_mode,
            spec.mode + " driver_mode");
        test_support::expect_equal(result.status, std::string("passed"),
            spec.mode + " status");

        if (spec.expect_stage_breakdown) {
            test_support::expect_true(!result.stage_breakdown.empty(),
                spec.mode + " has non-empty stage_breakdown");
        } else {
            test_support::expect_true(result.stage_breakdown.empty(),
                spec.mode + " has empty stage_breakdown");
        }
    }

    // Test unknown backend returns nullptr
    {
        auto unknown = benchmark::make_backend("nonexistent_backend");
        test_support::expect_true(unknown == nullptr, "unknown backend returns nullptr");
    }

    // Test stub backend still works
    {
        const auto summary =
            benchmark::make_stub_native_backend().run_compile_case("S1_BasePBR");
        test_support::expect_true(summary.total_us >= 0, "stub timing emitted");
        test_support::expect_equal(summary.api, std::string("Stub"), "stub api name");
    }

    std::cout << "graphics_backend_test: all checks completed\n";
    return test_support::finish();
}
