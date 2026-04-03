#pragma once

#include "driver/common/graphics_backend.h"
#include "driver/vulkan/vulkan_context.h"

namespace benchmark {

class RealVulkanBackend : public GraphicsBackend {
public:
    RealVulkanBackend();
    ~RealVulkanBackend() override;

    GraphicsResult run_compile_case(const std::string& workload_tier) override;

    bool is_real() const { return ctx_.is_available(); }

private:
    VulkanContext ctx_;
    bool initialized_ = false;
};

}  // namespace benchmark
