#pragma once

#include "driver/common/graphics_backend.h"

namespace benchmark {

class MetalBackend final : public GraphicsBackend {
public:
    GraphicsResult run_compile_case(const std::string& workload_tier) override;
};

}  // namespace benchmark
