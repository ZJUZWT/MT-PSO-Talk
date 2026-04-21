// ====================================================================
// vulkan_test.cpp – Vulkan PSO benchmark with colorWriteMask = 0
//
// This test creates a Vulkan graphics pipeline (PSO) where:
//   - The vertex shader does 5000 iterations of sin/cos (heavy ALU)
//   - The fragment shader simply passes through the result
//   - colorWriteMask is set to 0 in the PSO descriptor
//
// A smart Vulkan compiler will perform cross-stage DCE and eliminate
// the entire heavy computation because the output is provably unused.
//
// We measure pure GPU execution time using vkCmdWriteTimestamp.
// ====================================================================

#include "benchmark.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cassert>

// ── Helper macros ──────────────────────────────────────────────────
#define VK_CHECK(call)                                                     \
    do {                                                                   \
        VkResult _r = (call);                                              \
        if (_r != VK_SUCCESS) {                                            \
            std::cerr << "Vulkan error " << _r << " at " << __FILE__       \
                      << ":" << __LINE__ << "\n";                          \
            throw std::runtime_error("Vulkan call failed");                \
        }                                                                  \
    } while (0)

// ── Grid vertex data (32x32 grid = 1024 quads = 2048 triangles = 6144 vertices)
// This provides enough vertex invocations to amplify GPU shader workload.
static constexpr int GRID_SIZE = 32;
static constexpr int GRID_VERTEX_COUNT = GRID_SIZE * GRID_SIZE * 6; // 6 verts per quad

static void generateGridVertices(std::vector<float>& out) {
    out.clear();
    out.reserve(GRID_VERTEX_COUNT * 3);
    float step = 2.0f / GRID_SIZE; // covers [-1, 1]
    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            float x0 = -1.0f + x * step;
            float y0 = -1.0f + y * step;
            float x1 = x0 + step;
            float y1 = y0 + step;
            // Triangle 1
            out.push_back(x0); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x1); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x0); out.push_back(y1); out.push_back(0.0f);
            // Triangle 2
            out.push_back(x1); out.push_back(y0); out.push_back(0.0f);
            out.push_back(x1); out.push_back(y1); out.push_back(0.0f);
            out.push_back(x0); out.push_back(y1); out.push_back(0.0f);
        }
    }
}

// ── Vulkan context (all state in one struct for clarity) ───────────
struct VulkanContext {
    GLFWwindow*              window          = nullptr;
    VkInstance               instance        = VK_NULL_HANDLE;
    VkSurfaceKHR             surface         = VK_NULL_HANDLE;
    VkPhysicalDevice         physicalDevice  = VK_NULL_HANDLE;
    VkDevice                 device          = VK_NULL_HANDLE;
    VkQueue                  graphicsQueue   = VK_NULL_HANDLE;
    uint32_t                 queueFamily     = 0;
    VkCommandPool            commandPool     = VK_NULL_HANDLE;
    VkCommandBuffer          commandBuffer   = VK_NULL_HANDLE;
    VkRenderPass             renderPass      = VK_NULL_HANDLE;
    VkFramebuffer            framebuffer     = VK_NULL_HANDLE;
    VkImage                  colorImage      = VK_NULL_HANDLE;
    VkDeviceMemory           colorMemory     = VK_NULL_HANDLE;
    VkImageView              colorView       = VK_NULL_HANDLE;
    VkPipelineLayout         pipelineLayout  = VK_NULL_HANDLE;
    VkPipeline               pipeline        = VK_NULL_HANDLE;
    VkBuffer                 vertexBuffer    = VK_NULL_HANDLE;
    VkDeviceMemory           vertexMemory    = VK_NULL_HANDLE;
    VkQueryPool              queryPool       = VK_NULL_HANDLE;
    VkFence                  fence           = VK_NULL_HANDLE;
    float                    timestampPeriod = 0.0f;
};

// ── Forward declarations ───────────────────────────────────────────
static void createInstance(VulkanContext& ctx);
static void pickPhysicalDevice(VulkanContext& ctx);
static void createDevice(VulkanContext& ctx);
static void createCommandPool(VulkanContext& ctx);
static void createRenderTarget(VulkanContext& ctx, uint32_t w, uint32_t h);
static void createRenderPass(VulkanContext& ctx);
static void createFramebuffer(VulkanContext& ctx, uint32_t w, uint32_t h);
static void createPipeline(VulkanContext& ctx);
static void createBaselinePipeline(VulkanContext& ctx);
static void createVertexBuffer(VulkanContext& ctx, const std::vector<float>& vertices);
static void createQueryPool(VulkanContext& ctx);
static void cleanup(VulkanContext& ctx);
static uint32_t findMemoryType(VulkanContext& ctx, uint32_t filter, VkMemoryPropertyFlags props);

// ====================================================================
// Public entry point
// ====================================================================
BenchmarkResult runVulkanBenchmark(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend   = "Vulkan";
    result.testCase  = "ColorWriteMask=0 (PSO DCE test)";
    result.drawCalls = iterations;
    result.iterations = iterations;
    result.success   = false;
    result.gpuTimeMs = 0.0;

    // Generate grid vertices
    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    VulkanContext ctx{};

    try {
        // ── Init GLFW (no window shown – we render offscreen) ──────
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        ctx.window = glfwCreateWindow(64, 64, "VK Bench", nullptr, nullptr);

        createInstance(ctx);

        // We don't actually need a surface for offscreen rendering,
        // but some drivers require it for queue family detection.
        glfwCreateWindowSurface(ctx.instance, ctx.window, nullptr, &ctx.surface);

        pickPhysicalDevice(ctx);
        createDevice(ctx);
        createCommandPool(ctx);

        const uint32_t W = 64, H = 64;
        createRenderTarget(ctx, W, H);
        createRenderPass(ctx);
        createFramebuffer(ctx, W, H);
        createPipeline(ctx);
        createVertexBuffer(ctx, gridVertices);
        createQueryPool(ctx);

        // ── Create fence ───────────────────────────────────────────
        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        VK_CHECK(vkCreateFence(ctx.device, &fenceInfo, nullptr, &ctx.fence));

        // ── Allocate command buffer ────────────────────────────────
        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandPool        = ctx.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(ctx.device, &allocInfo, &ctx.commandBuffer));

        // ── Warm-up pass (eliminate cold-start overhead) ───────────
        {
            VkCommandBufferBeginInfo warmBegin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            warmBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &warmBegin));

            VkClearValue warmClear{};
            warmClear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            VkRenderPassBeginInfo warmRP{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            warmRP.renderPass        = ctx.renderPass;
            warmRP.framebuffer       = ctx.framebuffer;
            warmRP.renderArea.extent = {W, H};
            warmRP.clearValueCount   = 1;
            warmRP.pClearValues      = &warmClear;

            vkCmdBeginRenderPass(ctx.commandBuffer, &warmRP, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline);

            // Push constants: loopCount + seed
            struct { int loopCount; float seed; } warmPC = { loopCount, 1.23f };
            vkCmdPushConstants(ctx.commandBuffer, ctx.pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(warmPC), &warmPC);

            VkDeviceSize warmOff = 0;
            vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &warmOff);
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
            vkCmdEndRenderPass(ctx.commandBuffer);

            VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

            VkSubmitInfo warmSubmit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            warmSubmit.commandBufferCount = 1;
            warmSubmit.pCommandBuffers    = &ctx.commandBuffer;
            VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &warmSubmit, ctx.fence));
            VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));
            VK_CHECK(vkResetFences(ctx.device, 1, &ctx.fence));
            VK_CHECK(vkResetCommandBuffer(ctx.commandBuffer, 0));
        }

        // ── Record timed command buffer ────────────────────────────
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &beginInfo));

        vkCmdResetQueryPool(ctx.commandBuffer, ctx.queryPool, 0, 2);

        // Timestamp BEFORE
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            ctx.queryPool, 0);

        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        VkRenderPassBeginInfo rpBegin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpBegin.renderPass        = ctx.renderPass;
        rpBegin.framebuffer       = ctx.framebuffer;
        rpBegin.renderArea.extent = {W, H};
        rpBegin.clearValueCount   = 1;
        rpBegin.pClearValues      = &clearValue;

        vkCmdBeginRenderPass(ctx.commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline);

        // Push constants: loopCount + seed
        struct { int lc; float seed; } pc = { loopCount, 1.23f };
        vkCmdPushConstants(ctx.commandBuffer, ctx.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &offset);

        // Issue N draw calls to amplify the signal
        for (int i = 0; i < iterations; i++) {
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
        }

        vkCmdEndRenderPass(ctx.commandBuffer);

        // Timestamp AFTER
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            ctx.queryPool, 1);

        VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

        // ── Submit and wait ────────────────────────────────────────
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &ctx.commandBuffer;
        VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, ctx.fence));
        VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));

        // ── Read timestamps ────────────────────────────────────────
        uint64_t timestamps[2] = {0, 0};
        VK_CHECK(vkGetQueryPoolResults(ctx.device, ctx.queryPool, 0, 2,
                                       sizeof(timestamps), timestamps,
                                       sizeof(uint64_t),
                                       VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

        double nanoseconds = static_cast<double>(timestamps[1] - timestamps[0])
                             * static_cast<double>(ctx.timestampPeriod);
        result.gpuTimeMs = nanoseconds / 1e6;
        result.success   = true;

    } catch (const std::exception& e) {
        std::cerr << "[Vulkan] Error: " << e.what() << "\n";
    }

    cleanup(ctx);
    return result;
}

// ====================================================================
// Implementation details
// ====================================================================

static void createInstance(VulkanContext& ctx) {
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "ShaderCompileTest";
    appInfo.apiVersion       = VK_API_VERSION_1_1;

    uint32_t glfwExtCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ci.pApplicationInfo        = &appInfo;
    ci.enabledExtensionCount   = glfwExtCount;
    ci.ppEnabledExtensionNames = glfwExts;
    VK_CHECK(vkCreateInstance(&ci, nullptr, &ctx.instance));
}

static void pickPhysicalDevice(VulkanContext& ctx) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(ctx.instance, &count, nullptr);
    if (count == 0) throw std::runtime_error("No Vulkan physical devices found");

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(ctx.instance, &count, devices.data());

    // Prefer discrete GPU
    for (auto& d : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(d, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ctx.physicalDevice  = d;
            ctx.timestampPeriod = props.limits.timestampPeriod;
            std::cout << "[Vulkan] Using device: " << props.deviceName << "\n";
            return;
        }
    }
    // Fallback to first device
    VkPhysicalDeviceProperties props;
    ctx.physicalDevice = devices[0];
    vkGetPhysicalDeviceProperties(ctx.physicalDevice, &props);
    ctx.timestampPeriod = props.limits.timestampPeriod;
    std::cout << "[Vulkan] Using device: " << props.deviceName << "\n";
}

static void createDevice(VulkanContext& ctx) {
    uint32_t qfCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &qfCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfProps(qfCount);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx.physicalDevice, &qfCount, qfProps.data());

    for (uint32_t i = 0; i < qfCount; i++) {
        if (qfProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            ctx.queueFamily = i;
            break;
        }
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = ctx.queueFamily;
    qci.queueCount       = 1;
    qci.pQueuePriorities = &priority;

    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos    = &qci;

    VK_CHECK(vkCreateDevice(ctx.physicalDevice, &dci, nullptr, &ctx.device));
    vkGetDeviceQueue(ctx.device, ctx.queueFamily, 0, &ctx.graphicsQueue);
}

static void createCommandPool(VulkanContext& ctx) {
    VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    ci.queueFamilyIndex = ctx.queueFamily;
    ci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(ctx.device, &ci, nullptr, &ctx.commandPool));
}

static uint32_t findMemoryType(VulkanContext& ctx, uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type");
}

static void createRenderTarget(VulkanContext& ctx, uint32_t w, uint32_t h) {
    // Create a small offscreen color attachment
    VkImageCreateInfo imgCI{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imgCI.imageType     = VK_IMAGE_TYPE_2D;
    imgCI.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imgCI.extent        = {w, h, 1};
    imgCI.mipLevels     = 1;
    imgCI.arrayLayers   = 1;
    imgCI.samples       = VK_SAMPLE_COUNT_1_BIT;
    imgCI.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgCI.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imgCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VK_CHECK(vkCreateImage(ctx.device, &imgCI, nullptr, &ctx.colorImage));

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(ctx.device, ctx.colorImage, &memReq);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx, memReq.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK(vkAllocateMemory(ctx.device, &allocInfo, nullptr, &ctx.colorMemory));
    VK_CHECK(vkBindImageMemory(ctx.device, ctx.colorImage, ctx.colorMemory, 0));

    VkImageViewCreateInfo viewCI{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCI.image                           = ctx.colorImage;
    viewCI.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewCI.format                          = VK_FORMAT_R8G8B8A8_UNORM;
    viewCI.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCI.subresourceRange.baseMipLevel   = 0;
    viewCI.subresourceRange.levelCount     = 1;
    viewCI.subresourceRange.baseArrayLayer = 0;
    viewCI.subresourceRange.layerCount     = 1;
    VK_CHECK(vkCreateImageView(ctx.device, &viewCI, nullptr, &ctx.colorView));
}

static void createRenderPass(VulkanContext& ctx) {
    VkAttachmentDescription colorAtt{};
    colorAtt.format         = VK_FORMAT_R8G8B8A8_UNORM;
    colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE; // we don't read back
    colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{};
    colorRef.attachment = 0;
    colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorRef;

    VkRenderPassCreateInfo rpCI{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpCI.attachmentCount = 1;
    rpCI.pAttachments    = &colorAtt;
    rpCI.subpassCount    = 1;
    rpCI.pSubpasses      = &subpass;
    VK_CHECK(vkCreateRenderPass(ctx.device, &rpCI, nullptr, &ctx.renderPass));
}

static void createFramebuffer(VulkanContext& ctx, uint32_t w, uint32_t h) {
    VkFramebufferCreateInfo fbCI{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbCI.renderPass      = ctx.renderPass;
    fbCI.attachmentCount = 1;
    fbCI.pAttachments    = &ctx.colorView;
    fbCI.width           = w;
    fbCI.height          = h;
    fbCI.layers          = 1;
    VK_CHECK(vkCreateFramebuffer(ctx.device, &fbCI, nullptr, &ctx.framebuffer));
}

static void createPipeline(VulkanContext& ctx) {
    // ── Load SPIR-V ────────────────────────────────────────────────
    auto vertCode = readFile("shaders/spirv/heavy.vert.spv");
    auto fragCode = readFile("shaders/spirv/heavy.frag.spv");

    VkShaderModuleCreateInfo smCI{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    VkShaderModule vertModule, fragModule;

    smCI.codeSize = vertCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(vertCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &vertModule));

    smCI.codeSize = fragCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(fragCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &fragModule));

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName  = "main";

    // ── Vertex input ───────────────────────────────────────────────
    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.stride    = 3 * sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr{};
    attr.location = 0;
    attr.binding  = 0;
    attr.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attr.offset   = 0;

    VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount   = 1;
    vertexInput.pVertexBindingDescriptions      = &binding;
    vertexInput.vertexAttributeDescriptionCount = 1;
    vertexInput.pVertexAttributeDescriptions    = &attr;

    // ── Input assembly ─────────────────────────────────────────────
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // ── Viewport / scissor ─────────────────────────────────────────
    VkViewport viewport{0, 0, 64, 64, 0, 1};
    VkRect2D   scissor{{0, 0}, {64, 64}};

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    // ── Rasterization ──────────────────────────────────────────────
    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode    = VK_CULL_MODE_NONE;
    raster.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth   = 1.0f;

    // ── Multisample ────────────────────────────────────────────────
    VkPipelineMultisampleStateCreateInfo msaa{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // ── Color blend ────────────────────────────────────────────────
    // *** THIS IS THE KEY: colorWriteMask = 0 ***
    // The Vulkan compiler sees this at PSO creation time and can
    // perform aggressive cross-stage dead code elimination.
    VkPipelineColorBlendAttachmentState blendAtt{};
    blendAtt.colorWriteMask = 0;  // <── THE TRAP: no color output!
    blendAtt.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments    = &blendAtt;

    // ── Pipeline layout (with push constants) ──────────────────────
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset     = 0;
    pushRange.size       = sizeof(int) + sizeof(float); // loopCount + seed

    VkPipelineLayoutCreateInfo layoutCI{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCI.pushConstantRangeCount = 1;
    layoutCI.pPushConstantRanges    = &pushRange;
    VK_CHECK(vkCreatePipelineLayout(ctx.device, &layoutCI, nullptr, &ctx.pipelineLayout));

    // ── Create the graphics pipeline (PSO) ─────────────────────────
    VkGraphicsPipelineCreateInfo pipeCI{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeCI.stageCount          = 2;
    pipeCI.pStages             = stages;
    pipeCI.pVertexInputState   = &vertexInput;
    pipeCI.pInputAssemblyState = &inputAssembly;
    pipeCI.pViewportState      = &viewportState;
    pipeCI.pRasterizationState = &raster;
    pipeCI.pMultisampleState   = &msaa;
    pipeCI.pColorBlendState    = &blend;
    pipeCI.layout              = ctx.pipelineLayout;
    pipeCI.renderPass          = ctx.renderPass;
    pipeCI.subpass             = 0;

    VK_CHECK(vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeCI,
                                       nullptr, &ctx.pipeline));

    vkDestroyShaderModule(ctx.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx.device, fragModule, nullptr);

    std::cout << "[Vulkan] Pipeline created with colorWriteMask = 0\n";
}

static void createVertexBuffer(VulkanContext& ctx, const std::vector<float>& vertices) {
    VkDeviceSize bufSize = vertices.size() * sizeof(float);

    VkBufferCreateInfo bufCI{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufCI.size  = bufSize;
    bufCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VK_CHECK(vkCreateBuffer(ctx.device, &bufCI, nullptr, &ctx.vertexBuffer));

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(ctx.device, ctx.vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx, memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK(vkAllocateMemory(ctx.device, &allocInfo, nullptr, &ctx.vertexMemory));
    VK_CHECK(vkBindBufferMemory(ctx.device, ctx.vertexBuffer, ctx.vertexMemory, 0));

    void* data;
    VK_CHECK(vkMapMemory(ctx.device, ctx.vertexMemory, 0, bufSize, 0, &data));
    memcpy(data, vertices.data(), bufSize);
    vkUnmapMemory(ctx.device, ctx.vertexMemory);
}

static void createQueryPool(VulkanContext& ctx) {
    VkQueryPoolCreateInfo qpCI{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    qpCI.queryType  = VK_QUERY_TYPE_TIMESTAMP;
    qpCI.queryCount = 2;
    VK_CHECK(vkCreateQueryPool(ctx.device, &qpCI, nullptr, &ctx.queryPool));
}

static void cleanup(VulkanContext& ctx) {
    if (ctx.device) vkDeviceWaitIdle(ctx.device);

    auto D = [&](auto fn, auto handle) {
        if (handle) fn(ctx.device, handle, nullptr);
    };

    D(vkDestroyQueryPool,      ctx.queryPool);
    D(vkDestroyFence,          ctx.fence);
    D(vkDestroyBuffer,         ctx.vertexBuffer);
    D(vkFreeMemory,            ctx.vertexMemory);
    D(vkDestroyPipeline,       ctx.pipeline);
    D(vkDestroyPipelineLayout, ctx.pipelineLayout);
    D(vkDestroyFramebuffer,    ctx.framebuffer);
    D(vkDestroyRenderPass,     ctx.renderPass);
    D(vkDestroyImageView,      ctx.colorView);
    D(vkDestroyImage,          ctx.colorImage);
    D(vkFreeMemory,            ctx.colorMemory);
    D(vkDestroyCommandPool,    ctx.commandPool);

    if (ctx.device)   vkDestroyDevice(ctx.device, nullptr);
    if (ctx.surface)  vkDestroySurfaceKHR(ctx.instance, ctx.surface, nullptr);
    if (ctx.instance) vkDestroyInstance(ctx.instance, nullptr);

    if (ctx.window) {
        glfwDestroyWindow(ctx.window);
        glfwTerminate();
    }
}

// ====================================================================
// Heavy pipeline with colorWriteMask ENABLED (forces actual execution)
// ====================================================================
static VkPipeline       s_colorWritePipeline = VK_NULL_HANDLE;
static VkPipelineLayout s_colorWriteLayout   = VK_NULL_HANDLE;

static void createColorWritePipeline(VulkanContext& ctx) {
    auto vertCode = readFile("shaders/spirv/heavy.vert.spv");
    auto fragCode = readFile("shaders/spirv/heavy.frag.spv");

    VkShaderModuleCreateInfo smCI{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    VkShaderModule vertModule, fragModule;

    smCI.codeSize = vertCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(vertCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &vertModule));

    smCI.codeSize = fragCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(fragCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &fragModule));

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName  = "main";

    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.stride    = 3 * sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr{};
    attr.location = 0;
    attr.binding  = 0;
    attr.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attr.offset   = 0;

    VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount   = 1;
    vertexInput.pVertexBindingDescriptions      = &binding;
    vertexInput.vertexAttributeDescriptionCount = 1;
    vertexInput.pVertexAttributeDescriptions    = &attr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{0, 0, 64, 64, 0, 1};
    VkRect2D   scissor{{0, 0}, {64, 64}};

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode    = VK_CULL_MODE_NONE;
    raster.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo msaa{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // *** colorWriteMask FULLY ENABLED – GPU must actually compute and write ***
    VkPipelineColorBlendAttachmentState blendAtt{};
    blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAtt.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments    = &blendAtt;

    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset     = 0;
    pushRange.size       = sizeof(int) + sizeof(float);

    VkPipelineLayoutCreateInfo layoutCI{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCI.pushConstantRangeCount = 1;
    layoutCI.pPushConstantRanges    = &pushRange;
    VK_CHECK(vkCreatePipelineLayout(ctx.device, &layoutCI, nullptr, &s_colorWriteLayout));

    VkGraphicsPipelineCreateInfo pipeCI{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeCI.stageCount          = 2;
    pipeCI.pStages             = stages;
    pipeCI.pVertexInputState   = &vertexInput;
    pipeCI.pInputAssemblyState = &inputAssembly;
    pipeCI.pViewportState      = &viewportState;
    pipeCI.pRasterizationState = &raster;
    pipeCI.pMultisampleState   = &msaa;
    pipeCI.pColorBlendState    = &blend;
    pipeCI.layout              = s_colorWriteLayout;
    pipeCI.renderPass          = ctx.renderPass;
    pipeCI.subpass             = 0;

    VK_CHECK(vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeCI,
                                       nullptr, &s_colorWritePipeline));

    vkDestroyShaderModule(ctx.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx.device, fragModule, nullptr);

    std::cout << "[Vulkan] Pipeline created with colorWriteMask = RGBA (full write)\n";
}

// ====================================================================
// Heavy shader + colorWriteMask ENABLED benchmark
// ====================================================================
BenchmarkResult runVulkanBenchmarkColorWrite(int iterations, int loopCount) {
    BenchmarkResult result;
    result.backend   = "Vulkan";
    result.testCase  = "ColorWriteMask=RGBA (heavy shader, full write)";
    result.drawCalls = iterations;
    result.iterations = iterations;
    result.success   = false;
    result.gpuTimeMs = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    VulkanContext ctx{};

    try {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        ctx.window = glfwCreateWindow(64, 64, "VK ColorWrite", nullptr, nullptr);

        createInstance(ctx);
        glfwCreateWindowSurface(ctx.instance, ctx.window, nullptr, &ctx.surface);
        pickPhysicalDevice(ctx);
        createDevice(ctx);
        createCommandPool(ctx);

        const uint32_t W = 64, H = 64;
        createRenderTarget(ctx, W, H);
        createRenderPass(ctx);
        createFramebuffer(ctx, W, H);
        createColorWritePipeline(ctx);
        createVertexBuffer(ctx, gridVertices);
        createQueryPool(ctx);

        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        VK_CHECK(vkCreateFence(ctx.device, &fenceInfo, nullptr, &ctx.fence));

        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandPool        = ctx.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(ctx.device, &allocInfo, &ctx.commandBuffer));

        // Warm-up
        {
            VkCommandBufferBeginInfo warmBegin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            warmBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &warmBegin));

            VkClearValue warmClear{};
            warmClear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            VkRenderPassBeginInfo warmRP{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            warmRP.renderPass        = ctx.renderPass;
            warmRP.framebuffer       = ctx.framebuffer;
            warmRP.renderArea.extent = {W, H};
            warmRP.clearValueCount   = 1;
            warmRP.pClearValues      = &warmClear;

            vkCmdBeginRenderPass(ctx.commandBuffer, &warmRP, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_colorWritePipeline);

            struct { int loopCount; float seed; } warmPC = { loopCount, 1.23f };
            vkCmdPushConstants(ctx.commandBuffer, s_colorWriteLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(warmPC), &warmPC);

            VkDeviceSize warmOff = 0;
            vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &warmOff);
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
            vkCmdEndRenderPass(ctx.commandBuffer);

            VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

            VkSubmitInfo warmSubmit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            warmSubmit.commandBufferCount = 1;
            warmSubmit.pCommandBuffers    = &ctx.commandBuffer;
            VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &warmSubmit, ctx.fence));
            VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));
            VK_CHECK(vkResetFences(ctx.device, 1, &ctx.fence));
            VK_CHECK(vkResetCommandBuffer(ctx.commandBuffer, 0));
        }

        // Timed pass
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &beginInfo));

        vkCmdResetQueryPool(ctx.commandBuffer, ctx.queryPool, 0, 2);
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            ctx.queryPool, 0);

        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        VkRenderPassBeginInfo rpBegin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpBegin.renderPass        = ctx.renderPass;
        rpBegin.framebuffer       = ctx.framebuffer;
        rpBegin.renderArea.extent = {W, H};
        rpBegin.clearValueCount   = 1;
        rpBegin.pClearValues      = &clearValue;

        vkCmdBeginRenderPass(ctx.commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_colorWritePipeline);

        struct { int lc; float seed; } pc = { loopCount, 1.23f };
        vkCmdPushConstants(ctx.commandBuffer, s_colorWriteLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &offset);

        for (int i = 0; i < iterations; i++) {
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
        }

        vkCmdEndRenderPass(ctx.commandBuffer);
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            ctx.queryPool, 1);

        VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &ctx.commandBuffer;
        VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, ctx.fence));
        VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));

        uint64_t timestamps[2] = {0, 0};
        VK_CHECK(vkGetQueryPoolResults(ctx.device, ctx.queryPool, 0, 2,
                                       sizeof(timestamps), timestamps,
                                       sizeof(uint64_t),
                                       VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

        double nanoseconds = static_cast<double>(timestamps[1] - timestamps[0])
                             * static_cast<double>(ctx.timestampPeriod);
        result.gpuTimeMs = nanoseconds / 1e6;
        result.success   = true;

    } catch (const std::exception& e) {
        std::cerr << "[Vulkan ColorWrite] Error: " << e.what() << "\n";
    }

    // Clean up the extra pipeline/layout before full cleanup
    if (ctx.device) {
        if (s_colorWritePipeline) vkDestroyPipeline(ctx.device, s_colorWritePipeline, nullptr);
        if (s_colorWriteLayout)   vkDestroyPipelineLayout(ctx.device, s_colorWriteLayout, nullptr);
        s_colorWritePipeline = VK_NULL_HANDLE;
        s_colorWriteLayout   = VK_NULL_HANDLE;
    }

    cleanup(ctx);
    return result;
}

// ====================================================================
// Baseline pipeline: uses baseline shaders + normal colorWriteMask
// ====================================================================
static void createBaselinePipeline(VulkanContext& ctx) {
    auto vertCode = readFile("shaders/spirv/baseline.vert.spv");
    auto fragCode = readFile("shaders/spirv/baseline.frag.spv");

    VkShaderModuleCreateInfo smCI{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    VkShaderModule vertModule, fragModule;

    smCI.codeSize = vertCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(vertCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &vertModule));

    smCI.codeSize = fragCode.size();
    smCI.pCode    = reinterpret_cast<const uint32_t*>(fragCode.data());
    VK_CHECK(vkCreateShaderModule(ctx.device, &smCI, nullptr, &fragModule));

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertModule;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragModule;
    stages[1].pName  = "main";

    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.stride    = 3 * sizeof(float);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attr{};
    attr.location = 0;
    attr.binding  = 0;
    attr.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attr.offset   = 0;

    VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount   = 1;
    vertexInput.pVertexBindingDescriptions      = &binding;
    vertexInput.vertexAttributeDescriptionCount = 1;
    vertexInput.pVertexAttributeDescriptions    = &attr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{0, 0, 64, 64, 0, 1};
    VkRect2D   scissor{{0, 0}, {64, 64}};

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode    = VK_CULL_MODE_NONE;
    raster.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    raster.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo msaa{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msaa.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Normal colorWriteMask – actually writes color
    VkPipelineColorBlendAttachmentState blendAtt{};
    blendAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAtt.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments    = &blendAtt;

    // Same push constant layout for compatibility
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset     = 0;
    pushRange.size       = sizeof(int) + sizeof(float);

    VkPipelineLayoutCreateInfo layoutCI{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCI.pushConstantRangeCount = 1;
    layoutCI.pPushConstantRanges    = &pushRange;
    VK_CHECK(vkCreatePipelineLayout(ctx.device, &layoutCI, nullptr, &ctx.pipelineLayout));

    VkGraphicsPipelineCreateInfo pipeCI{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeCI.stageCount          = 2;
    pipeCI.pStages             = stages;
    pipeCI.pVertexInputState   = &vertexInput;
    pipeCI.pInputAssemblyState = &inputAssembly;
    pipeCI.pViewportState      = &viewportState;
    pipeCI.pRasterizationState = &raster;
    pipeCI.pMultisampleState   = &msaa;
    pipeCI.pColorBlendState    = &blend;
    pipeCI.layout              = ctx.pipelineLayout;
    pipeCI.renderPass          = ctx.renderPass;
    pipeCI.subpass             = 0;

    VK_CHECK(vkCreateGraphicsPipelines(ctx.device, VK_NULL_HANDLE, 1, &pipeCI,
                                       nullptr, &ctx.pipeline));

    vkDestroyShaderModule(ctx.device, vertModule, nullptr);
    vkDestroyShaderModule(ctx.device, fragModule, nullptr);

    std::cout << "[Vulkan] Baseline pipeline created (empty shader, normal colorWriteMask)\n";
}

// ====================================================================
// Baseline benchmark: empty shaders + normal colorWriteMask
// ====================================================================
BenchmarkResult runVulkanBaseline(int iterations) {
    BenchmarkResult result;
    result.backend    = "Vulkan";
    result.testCase   = "Baseline (empty shader, normal colorWriteMask)";
    result.drawCalls  = iterations;
    result.iterations = iterations;
    result.success    = false;
    result.gpuTimeMs  = 0.0;

    std::vector<float> gridVertices;
    generateGridVertices(gridVertices);

    VulkanContext ctx{};

    try {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        ctx.window = glfwCreateWindow(64, 64, "VK Baseline", nullptr, nullptr);

        createInstance(ctx);
        glfwCreateWindowSurface(ctx.instance, ctx.window, nullptr, &ctx.surface);
        pickPhysicalDevice(ctx);
        createDevice(ctx);
        createCommandPool(ctx);

        const uint32_t W = 64, H = 64;
        createRenderTarget(ctx, W, H);
        createRenderPass(ctx);
        createFramebuffer(ctx, W, H);
        createBaselinePipeline(ctx);
        createVertexBuffer(ctx, gridVertices);
        createQueryPool(ctx);

        VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        VK_CHECK(vkCreateFence(ctx.device, &fenceInfo, nullptr, &ctx.fence));

        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.commandPool        = ctx.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(ctx.device, &allocInfo, &ctx.commandBuffer));

        // Warm-up
        {
            VkCommandBufferBeginInfo warmBegin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            warmBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &warmBegin));

            VkClearValue warmClear{};
            warmClear.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            VkRenderPassBeginInfo warmRP{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            warmRP.renderPass        = ctx.renderPass;
            warmRP.framebuffer       = ctx.framebuffer;
            warmRP.renderArea.extent = {W, H};
            warmRP.clearValueCount   = 1;
            warmRP.pClearValues      = &warmClear;

            vkCmdBeginRenderPass(ctx.commandBuffer, &warmRP, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline);

            struct { int loopCount; float seed; } warmPC = { 0, 0.0f };
            vkCmdPushConstants(ctx.commandBuffer, ctx.pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(warmPC), &warmPC);

            VkDeviceSize warmOff = 0;
            vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &warmOff);
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
            vkCmdEndRenderPass(ctx.commandBuffer);

            VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

            VkSubmitInfo warmSubmit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
            warmSubmit.commandBufferCount = 1;
            warmSubmit.pCommandBuffers    = &ctx.commandBuffer;
            VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &warmSubmit, ctx.fence));
            VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));
            VK_CHECK(vkResetFences(ctx.device, 1, &ctx.fence));
            VK_CHECK(vkResetCommandBuffer(ctx.commandBuffer, 0));
        }

        // Timed pass
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(ctx.commandBuffer, &beginInfo));

        vkCmdResetQueryPool(ctx.commandBuffer, ctx.queryPool, 0, 2);
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            ctx.queryPool, 0);

        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        VkRenderPassBeginInfo rpBegin{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
        rpBegin.renderPass        = ctx.renderPass;
        rpBegin.framebuffer       = ctx.framebuffer;
        rpBegin.renderArea.extent = {W, H};
        rpBegin.clearValueCount   = 1;
        rpBegin.pClearValues      = &clearValue;

        vkCmdBeginRenderPass(ctx.commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(ctx.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.pipeline);

        struct { int lc; float seed; } pc = { 0, 0.0f };
        vkCmdPushConstants(ctx.commandBuffer, ctx.pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(ctx.commandBuffer, 0, 1, &ctx.vertexBuffer, &offset);

        for (int i = 0; i < iterations; i++) {
            vkCmdDraw(ctx.commandBuffer, GRID_VERTEX_COUNT, 1, 0, 0);
        }

        vkCmdEndRenderPass(ctx.commandBuffer);
        vkCmdWriteTimestamp(ctx.commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            ctx.queryPool, 1);

        VK_CHECK(vkEndCommandBuffer(ctx.commandBuffer));

        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers    = &ctx.commandBuffer;
        VK_CHECK(vkQueueSubmit(ctx.graphicsQueue, 1, &submitInfo, ctx.fence));
        VK_CHECK(vkWaitForFences(ctx.device, 1, &ctx.fence, VK_TRUE, UINT64_MAX));

        uint64_t timestamps[2] = {0, 0};
        VK_CHECK(vkGetQueryPoolResults(ctx.device, ctx.queryPool, 0, 2,
                                       sizeof(timestamps), timestamps,
                                       sizeof(uint64_t),
                                       VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

        double nanoseconds = static_cast<double>(timestamps[1] - timestamps[0])
                             * static_cast<double>(ctx.timestampPeriod);
        result.gpuTimeMs = nanoseconds / 1e6;
        result.success   = true;

    } catch (const std::exception& e) {
        std::cerr << "[Vulkan Baseline] Error: " << e.what() << "\n";
    }

    cleanup(ctx);
    return result;
}
