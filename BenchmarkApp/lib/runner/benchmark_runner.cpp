#include "runner/benchmark_runner.h"

namespace benchmark {

namespace {

std::vector<BenchmarkCase> make_graphics_plan(
    const std::string& mode,
    const std::string& api,
    const std::string& driver_mode) {
    return {
        BenchmarkCase{mode, api, driver_mode, "S1_BasePBR", "", false},
        BenchmarkCase{mode, api, driver_mode, "S2_MaterialLayered", "", false},
        BenchmarkCase{mode, api, driver_mode, "S3_FeatureHeavy", "", false},
        BenchmarkCase{mode, api, driver_mode, "S4_PermutationStress", "", false},
    };
}

std::vector<BenchmarkCase> make_compression_plan(const std::string& mode) {
    return {
        BenchmarkCase{mode, "", "", "", "zstd", true},
        BenchmarkCase{mode, "", "", "", "lz4", true},
        BenchmarkCase{mode, "", "", "", "zlib", true},
        BenchmarkCase{mode, "", "", "", "snappy", true},
    };
}

}  // namespace

std::vector<std::string> BenchmarkRunner::all_modes() {
    return {
        "android_mesa_opengl",
        "android_mesa_vulkan",
        "android_native_opengl",
        "android_native_vulkan",
        "ios_native_metal",
        "android_compression",
        "ios_compression",
        "windows_compression",
    };
}

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

    if (mode == "android_compression") {
        return make_compression_plan(mode);
    }

    if (mode == "ios_compression") {
        return make_compression_plan(mode);
    }

    if (mode == "windows_compression") {
        return make_compression_plan(mode);
    }

    return {};
}

std::vector<BenchmarkCase> BenchmarkRunner::plan_all() const {
    std::vector<BenchmarkCase> all;
    for (const auto& mode : all_modes()) {
        auto cases = plan_for_mode(mode);
        all.insert(all.end(), cases.begin(), cases.end());
    }
    return all;
}

}  // namespace benchmark
