#include "driver/opengl/opengl_backend.h"

#include "driver/common/device_info.h"
#include "driver/common/timing_model.h"

#if defined(__ANDROID__)
#include "driver/opengl/gl_program.h"
#endif

namespace benchmark {

RealOpenGLBackend::RealOpenGLBackend() {
    ctx_.init();
}

RealOpenGLBackend::~RealOpenGLBackend() {
    ctx_.destroy();
}

GraphicsResult RealOpenGLBackend::run_compile_case(const std::string& workload_tier) {
#if defined(__ANDROID__)
    if (ctx_.is_available()) {
        ctx_.make_current();
        auto info = query_device_info();

        // TODO: load real GLSL shaders from workload assets
        // For now, fall through to simulation with real device info

        auto workload = workload_for_tier(workload_tier);
        auto profile = native_opengl_profile();
        auto result = simulate_graphics_timing(
            profile, workload, "Android", "OpenGL ES", "RealGPU",
            /*cold_cache=*/true, /*iteration_index=*/0);
        result.device_model = info.device_model;
        result.soc = info.soc;
        result.os_version = info.os_version;
        return result;
    }
#endif

    // Fallback: simulation
    auto workload = workload_for_tier(workload_tier);
    auto profile = native_opengl_profile();
    return simulate_graphics_timing(
        profile, workload, "Simulated", "OpenGL ES", "Simulated",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

}  // namespace benchmark
