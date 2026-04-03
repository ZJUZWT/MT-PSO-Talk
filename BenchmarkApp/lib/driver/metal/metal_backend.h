#pragma once

#include "driver/common/graphics_backend.h"
#include "driver/metal/metal_context.h"

namespace benchmark {

class RealMetalBackend : public GraphicsBackend {
public:
    RealMetalBackend();
    ~RealMetalBackend() override;

    GraphicsResult run_compile_case(const std::string& workload_tier) override;

    bool is_real() const { return ctx_.is_available(); }

private:
    MetalContext ctx_;
    bool initialized_ = false;
};

}  // namespace benchmark
