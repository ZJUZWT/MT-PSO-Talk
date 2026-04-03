#include "driver/mesa/mesa_opengl_backend.h"

#include "driver/common/graphics_registry.h"
#include "driver/common/timing_model.h"

namespace benchmark {

GraphicsResult MesaOpenGLBackend::run_compile_case(
    const std::string& workload_tier) {
    auto workload = workload_for_tier(workload_tier);
    auto profile = mesa_opengl_profile();
    return simulate_graphics_timing(
        profile, workload, "Android", "OpenGL", "Mesa",
        /*cold_cache=*/true, /*iteration_index=*/0);
}

// Self-registration
static const bool _reg_gfx_mesa_opengl = (GraphicsBackendRegistry::instance().add(
    {"mesa_opengl", "Android", "OpenGL", "Mesa", mesa_opengl_profile}), true);

}  // namespace benchmark
