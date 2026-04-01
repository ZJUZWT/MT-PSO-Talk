#include "Runner/benchmark_runner.h"

namespace benchmark {

namespace {

std::vector<BenchmarkCase> make_graphics_plan(
    const std::string& mode,
    const std::string& api,
    const std::string& driver_mode) {
    return {
        BenchmarkCase{mode, api, driver_mode, "S1_BasePBR"},
        BenchmarkCase{mode, api, driver_mode, "S4_PermutationStress"},
    };
}

}  // namespace

std::vector<BenchmarkCase> BenchmarkRunner::plan_for_mode(
    const std::string& mode) const {
    if (mode == "android_mesa_opengl") {
        return make_graphics_plan(mode, "OpenGL", "Mesa");
    }

    if (mode == "android_mesa_vulkan") {
        return make_graphics_plan(mode, "Vulkan", "Mesa");
    }

    if (mode == "android_native_opengl") {
        return make_graphics_plan(mode, "OpenGL", "Native");
    }

    if (mode == "android_native_vulkan") {
        return make_graphics_plan(mode, "Vulkan", "Native");
    }

    if (mode == "ios_native_metal") {
        return make_graphics_plan(mode, "Metal", "Native");
    }

    return {};
}

}  // namespace benchmark
