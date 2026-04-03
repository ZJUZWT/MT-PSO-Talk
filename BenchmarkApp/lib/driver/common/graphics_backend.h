#pragma once

#include <memory>
#include <string>

#include "results/schema/result_types.h"

namespace benchmark {

class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;
    virtual GraphicsResult run_compile_case(const std::string& workload_tier) = 0;
};

GraphicsBackend& make_stub_native_backend();

// Factory for all backends
// mode: mesa_opengl, mesa_vulkan, native_opengl, native_vulkan, metal
std::unique_ptr<GraphicsBackend> make_backend(const std::string& mode);

}  // namespace benchmark
