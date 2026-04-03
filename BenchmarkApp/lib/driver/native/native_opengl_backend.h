#pragma once

#include "driver/common/graphics_backend.h"

namespace benchmark {

class NativeOpenGLBackend final : public GraphicsBackend {
public:
    GraphicsResult run_compile_case(const std::string& workload_tier) override;
};

}  // namespace benchmark
