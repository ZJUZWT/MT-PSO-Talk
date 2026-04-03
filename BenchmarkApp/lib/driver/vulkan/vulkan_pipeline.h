#pragma once

#include <cstdint>
#include <string>
#include <vector>

#if defined(__ANDROID__) || defined(HAVE_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace benchmark {

struct VulkanCompileResult {
    int64_t create_shader_module_us = 0;
    int64_t create_pipeline_layout_us = 0;
    int64_t create_graphics_pipeline_us = 0;
    int64_t total_us = 0;
    std::string status = "passed";
};

#if defined(__ANDROID__) || defined(HAVE_VULKAN)

VulkanCompileResult compile_vulkan_pipeline(
    VkDevice device,
    const std::vector<uint8_t>& vert_spv,
    const std::vector<uint8_t>& frag_spv,
    VkRenderPass render_pass,
    VkPipelineCache cache);

std::vector<uint8_t> compile_glsl_to_spirv(
    const std::string& source, bool is_vertex);

VkRenderPass create_minimal_render_pass(VkDevice device);

#endif

}  // namespace benchmark
