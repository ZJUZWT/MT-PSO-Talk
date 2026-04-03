#pragma once

#include "driver/common/graphics_backend.h"
#include "driver/opengl/egl_context.h"

namespace benchmark {

class RealOpenGLBackend : public GraphicsBackend {
public:
    RealOpenGLBackend();
    ~RealOpenGLBackend() override;

    GraphicsResult run_compile_case(const std::string& workload_tier) override;

    bool is_real() const { return ctx_.is_available(); }

private:
    EGLHeadlessContext ctx_;
    bool initialized_ = false;
};

}  // namespace benchmark
