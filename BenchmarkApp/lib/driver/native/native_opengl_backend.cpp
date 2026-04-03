#include "driver/native/native_opengl_backend.h"

#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"

namespace benchmark {

GraphicsResult NativeOpenGLBackend::run_compile_case(
    const std::string& workload_tier) {
    auto workload = workload_for_tier(workload_tier);
    auto profile = native_opengl_profile();
    return simulate_graphics_timing(
        profile, workload, "Android", "OpenGL", "Native",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

// Self-registration
static const bool _reg_gfx_native_opengl = (GraphicsBackendRegistry::instance().add(
    {"native_opengl", "Android", "OpenGL", "Native", native_opengl_profile}), true);

}  // namespace benchmark
