#include "driver/vulkan/vulkan_pipeline.h"

#include "driver/common/timing_scope.h"

#if defined(__ANDROID__) || defined(HAVE_VULKAN)

namespace benchmark {

VkRenderPass create_minimal_render_pass(VkDevice device) {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    VkRenderPassCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = 1;
    ci.pAttachments = &color_attachment;
    ci.subpassCount = 1;
    ci.pSubpasses = &subpass;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    vkCreateRenderPass(device, &ci, nullptr, &render_pass);
    return render_pass;
}

std::vector<uint8_t> compile_glsl_to_spirv(const std::string& /*source*/, bool /*is_vertex*/) {
    // Requires shaderc or external tool. Return empty to indicate failure.
    return {};
}

VulkanCompileResult compile_vulkan_pipeline(
    VkDevice device,
    const std::vector<uint8_t>& vert_spv,
    const std::vector<uint8_t>& frag_spv,
    VkRenderPass render_pass,
    VkPipelineCache cache) {

    VulkanCompileResult result;
    int64_t vert_module_us = 0, frag_module_us = 0;

    VkShaderModule vert_module = VK_NULL_HANDLE;
    VkShaderModule frag_module = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    // 1. Create vertex shader module
    {
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = vert_spv.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(vert_spv.data());

        TimingScope scope(vert_module_us);
        if (vkCreateShaderModule(device, &ci, nullptr, &vert_module) != VK_SUCCESS) {
            result.status = "failed_vert_module";
            return result;
        }
    }

    // 2. Create fragment shader module
    {
        VkShaderModuleCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ci.codeSize = frag_spv.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(frag_spv.data());

        TimingScope scope(frag_module_us);
        if (vkCreateShaderModule(device, &ci, nullptr, &frag_module) != VK_SUCCESS) {
            result.status = "failed_frag_module";
            vkDestroyShaderModule(device, vert_module, nullptr);
            return result;
        }
    }

    result.create_shader_module_us = vert_module_us + frag_module_us;

    // 3. Create pipeline layout
    {
        VkPipelineLayoutCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        TimingScope scope(result.create_pipeline_layout_us);
        if (vkCreatePipelineLayout(device, &ci, nullptr, &layout) != VK_SUCCESS) {
            result.status = "failed_layout";
            vkDestroyShaderModule(device, vert_module, nullptr);
            vkDestroyShaderModule(device, frag_module, nullptr);
            return result;
        }
    }

    // 4. Create graphics pipeline
    {
        VkPipelineShaderStageCreateInfo stages[2]{};
        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        stages[0].module = vert_module;
        stages[0].pName = "main";
        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stages[1].module = frag_module;
        stages[1].pName = "main";

        VkPipelineVertexInputStateCreateInfo vertex_input{};
        vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport viewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};
        VkRect2D scissor{{0, 0}, {1, 1}};

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState blend_attachment{};
        blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend{};
        color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend.attachmentCount = 1;
        color_blend.pAttachments = &blend_attachment;

        VkGraphicsPipelineCreateInfo pipeline_ci{};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_ci.stageCount = 2;
        pipeline_ci.pStages = stages;
        pipeline_ci.pVertexInputState = &vertex_input;
        pipeline_ci.pInputAssemblyState = &input_assembly;
        pipeline_ci.pViewportState = &viewport_state;
        pipeline_ci.pRasterizationState = &rasterizer;
        pipeline_ci.pMultisampleState = &multisampling;
        pipeline_ci.pColorBlendState = &color_blend;
        pipeline_ci.layout = layout;
        pipeline_ci.renderPass = render_pass;
        pipeline_ci.subpass = 0;

        TimingScope scope(result.create_graphics_pipeline_us);
        if (vkCreateGraphicsPipelines(device, cache, 1, &pipeline_ci, nullptr, &pipeline) != VK_SUCCESS) {
            result.status = "failed_pipeline";
        }
    }

    result.total_us = result.create_shader_module_us +
                      result.create_pipeline_layout_us +
                      result.create_graphics_pipeline_us;

    // Cleanup
    if (pipeline != VK_NULL_HANDLE) vkDestroyPipeline(device, pipeline, nullptr);
    if (layout != VK_NULL_HANDLE) vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyShaderModule(device, frag_module, nullptr);
    vkDestroyShaderModule(device, vert_module, nullptr);

    return result;
}

}  // namespace benchmark

#endif  // __ANDROID__ || HAVE_VULKAN
