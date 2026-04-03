#include "driver/common/graphics_backend.h"

#include "driver/mesa/mesa_opengl_backend.h"
#include "driver/mesa/mesa_vulkan_backend.h"
#include "driver/native/metal_backend.h"
#include "driver/native/native_opengl_backend.h"
#include "driver/native/native_vulkan_backend.h"

namespace benchmark {

std::unique_ptr<GraphicsBackend> make_backend(const std::string& mode) {
    if (mode == "mesa_opengl") return std::make_unique<MesaOpenGLBackend>();
    if (mode == "mesa_vulkan") return std::make_unique<MesaVulkanBackend>();
    if (mode == "native_opengl") return std::make_unique<NativeOpenGLBackend>();
    if (mode == "native_vulkan") return std::make_unique<NativeVulkanBackend>();
    if (mode == "metal") return std::make_unique<MetalBackend>();
    return nullptr;
}

}  // namespace benchmark
