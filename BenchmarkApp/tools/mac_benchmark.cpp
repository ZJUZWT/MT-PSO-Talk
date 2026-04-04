// Mac PSO Compilation Benchmark — Mesa Lavapipe Vulkan + Metal
// Uses dlopen to load Lavapipe ICD directly; no link-time Vulkan dependency.
// Build via CMake: cmake --build build --target mac_benchmark

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <unistd.h>

#include <vulkan/vulkan.h>

// Forward declaration from mac_benchmark_metal.mm
struct MetalTierResult {
    std::string tier;
    int64_t compile_us;
    int64_t pipeline_us;
    int64_t total_us;
    bool ok;
};
extern std::vector<MetalTierResult> run_metal_all_tiers();

// ========== Timing ==========
static int64_t now_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// ========== File I/O ==========
static std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return {};
    auto sz = f.tellg();
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.seekg(0);
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    return buf;
}

// ========== Vulkan function pointers (loaded at runtime) ==========
static PFN_vkGetInstanceProcAddr pfn_vkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr   pfn_vkGetDeviceProcAddr;

static PFN_vkCreateInstance                     pfn_vkCreateInstance;
static PFN_vkDestroyInstance                    pfn_vkDestroyInstance;
static PFN_vkEnumeratePhysicalDevices           pfn_vkEnumeratePhysicalDevices;
static PFN_vkGetPhysicalDeviceProperties        pfn_vkGetPhysicalDeviceProperties;
static PFN_vkGetPhysicalDeviceQueueFamilyProperties pfn_vkGetPhysicalDeviceQueueFamilyProperties;
static PFN_vkCreateDevice                       pfn_vkCreateDevice;
static PFN_vkDestroyDevice                      pfn_vkDestroyDevice;
static PFN_vkCreateShaderModule                 pfn_vkCreateShaderModule;
static PFN_vkDestroyShaderModule                pfn_vkDestroyShaderModule;
static PFN_vkCreateRenderPass                   pfn_vkCreateRenderPass;
static PFN_vkDestroyRenderPass                  pfn_vkDestroyRenderPass;
static PFN_vkCreatePipelineLayout               pfn_vkCreatePipelineLayout;
static PFN_vkDestroyPipelineLayout              pfn_vkDestroyPipelineLayout;
static PFN_vkCreateGraphicsPipelines            pfn_vkCreateGraphicsPipelines;
static PFN_vkDestroyPipeline                    pfn_vkDestroyPipeline;
static PFN_vkCreatePipelineCache                pfn_vkCreatePipelineCache;
static PFN_vkDestroyPipelineCache               pfn_vkDestroyPipelineCache;
static PFN_vkCreateDescriptorSetLayout          pfn_vkCreateDescriptorSetLayout;
static PFN_vkDestroyDescriptorSetLayout         pfn_vkDestroyDescriptorSetLayout;

static bool load_vulkan_icd(const char* lib_path) {
    void* lib = dlopen(lib_path, RTLD_NOW | RTLD_LOCAL);
    if (!lib) {
        std::cerr << "  dlopen failed: " << dlerror() << "\n";
        return false;
    }
    auto icdGetAddr = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vk_icdGetInstanceProcAddr");
    if (!icdGetAddr) {
        std::cerr << "  No vk_icdGetInstanceProcAddr\n";
        return false;
    }
    pfn_vkGetInstanceProcAddr = icdGetAddr;
    pfn_vkCreateInstance = (PFN_vkCreateInstance)icdGetAddr(VK_NULL_HANDLE, "vkCreateInstance");
    pfn_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)icdGetAddr(VK_NULL_HANDLE, "vkEnumeratePhysicalDevices");
    return pfn_vkCreateInstance != nullptr;
}

static void load_instance_funcs(VkInstance inst) {
    auto g = pfn_vkGetInstanceProcAddr;
    pfn_vkDestroyInstance                    = (PFN_vkDestroyInstance)g(inst, "vkDestroyInstance");
    pfn_vkEnumeratePhysicalDevices           = (PFN_vkEnumeratePhysicalDevices)g(inst, "vkEnumeratePhysicalDevices");
    pfn_vkGetPhysicalDeviceProperties        = (PFN_vkGetPhysicalDeviceProperties)g(inst, "vkGetPhysicalDeviceProperties");
    pfn_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)g(inst, "vkGetPhysicalDeviceQueueFamilyProperties");
    pfn_vkCreateDevice                       = (PFN_vkCreateDevice)g(inst, "vkCreateDevice");
    pfn_vkGetDeviceProcAddr                  = (PFN_vkGetDeviceProcAddr)g(inst, "vkGetDeviceProcAddr");
}

static void load_device_funcs(VkDevice dev) {
    auto g = pfn_vkGetDeviceProcAddr;
    pfn_vkDestroyDevice          = (PFN_vkDestroyDevice)g(dev, "vkDestroyDevice");
    pfn_vkCreateShaderModule     = (PFN_vkCreateShaderModule)g(dev, "vkCreateShaderModule");
    pfn_vkDestroyShaderModule    = (PFN_vkDestroyShaderModule)g(dev, "vkDestroyShaderModule");
    pfn_vkCreateRenderPass       = (PFN_vkCreateRenderPass)g(dev, "vkCreateRenderPass");
    pfn_vkDestroyRenderPass      = (PFN_vkDestroyRenderPass)g(dev, "vkDestroyRenderPass");
    pfn_vkCreatePipelineLayout   = (PFN_vkCreatePipelineLayout)g(dev, "vkCreatePipelineLayout");
    pfn_vkDestroyPipelineLayout  = (PFN_vkDestroyPipelineLayout)g(dev, "vkDestroyPipelineLayout");
    pfn_vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)g(dev, "vkCreateGraphicsPipelines");
    pfn_vkDestroyPipeline        = (PFN_vkDestroyPipeline)g(dev, "vkDestroyPipeline");
    pfn_vkCreatePipelineCache    = (PFN_vkCreatePipelineCache)g(dev, "vkCreatePipelineCache");
    pfn_vkDestroyPipelineCache   = (PFN_vkDestroyPipelineCache)g(dev, "vkDestroyPipelineCache");
    pfn_vkCreateDescriptorSetLayout  = (PFN_vkCreateDescriptorSetLayout)g(dev, "vkCreateDescriptorSetLayout");
    pfn_vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)g(dev, "vkDestroyDescriptorSetLayout");
}

// ========== Tier definition ==========
struct ShaderTier {
    std::string name;
    std::string display_name;
    std::string vert_filename;
    std::string frag_filename;
};

static const ShaderTier kTiers[] = {
    { "S1", "S1 BasePBR",           "s1_base_pbr.vert.spv",          "s1_base_pbr.frag.spv"          },
    { "S2", "S2 MaterialLayered",   "s2_material_layered.vert.spv",  "s2_material_layered.frag.spv"  },
    { "S3", "S3 FeatureHeavy",      "s3_feature_heavy.vert.spv",     "s3_feature_heavy.frag.spv"     },
    { "S4", "S4 PermutationStress", "s4_permutation_stress.vert.spv","s4_permutation_stress.frag.spv"},
};
static constexpr int kNumTiers = 4;

// ========== Tier benchmark result ==========
struct TierResult {
    std::string name;
    int64_t cold_us   = 0;
    int64_t warm_us   = 0;
    double  speedup   = 0.0;
    size_t  spv_bytes = 0;
    bool    ok        = false;
};

// ========== Create one pipeline, return vkCreateGraphicsPipelines time in us ==========
static int64_t create_pipeline_timed(
    VkDevice device,
    const std::vector<uint8_t>& vert_spv,
    const std::vector<uint8_t>& frag_spv,
    VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    VkPipelineCache cache,
    VkPipeline* out_pipeline)
{
    VkShaderModuleCreateInfo smCI{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    VkShaderModule vertMod, fragMod;

    smCI.codeSize = vert_spv.size();
    smCI.pCode = reinterpret_cast<const uint32_t*>(vert_spv.data());
    if (pfn_vkCreateShaderModule(device, &smCI, nullptr, &vertMod) != VK_SUCCESS)
        return -1;

    smCI.codeSize = frag_spv.size();
    smCI.pCode = reinterpret_cast<const uint32_t*>(frag_spv.data());
    if (pfn_vkCreateShaderModule(device, &smCI, nullptr, &fragMod) != VK_SUCCESS) {
        pfn_vkDestroyShaderModule(device, vertMod, nullptr);
        return -1;
    }

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertMod;
    stages[0].pName  = "main";
    stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragMod;
    stages[1].pName  = "main";

    // Vertex input: 1 binding (stride=32), 3 attributes: position, normal, uv
    VkVertexInputBindingDescription binding{};
    binding.binding   = 0;
    binding.stride    = 32;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrs[3]{};
    attrs[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};   // position
    attrs[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12};  // normal
    attrs[2] = {2, 0, VK_FORMAT_R32G32_SFLOAT,    24};  // uv

    VkPipelineVertexInputStateCreateInfo viCI{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    viCI.vertexBindingDescriptionCount   = 1;
    viCI.pVertexBindingDescriptions      = &binding;
    viCI.vertexAttributeDescriptionCount = 3;
    viCI.pVertexAttributeDescriptions    = attrs;

    VkPipelineInputAssemblyStateCreateInfo iaCI{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    iaCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp{0, 0, 800, 600, 0, 1};
    VkRect2D scissor{{0, 0}, {800, 600}};
    VkPipelineViewportStateCreateInfo vpCI{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vpCI.viewportCount = 1; vpCI.pViewports = &vp;
    vpCI.scissorCount  = 1; vpCI.pScissors  = &scissor;

    VkPipelineRasterizationStateCreateInfo rsCI{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rsCI.polygonMode = VK_POLYGON_MODE_FILL;
    rsCI.cullMode    = VK_CULL_MODE_BACK_BIT;
    rsCI.frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rsCI.lineWidth   = 1.0f;

    VkPipelineMultisampleStateCreateInfo msCI{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cbAtt{};
    cbAtt.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cbCI{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cbCI.attachmentCount = 1;
    cbCI.pAttachments    = &cbAtt;

    VkGraphicsPipelineCreateInfo pipeCI{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeCI.stageCount          = 2;
    pipeCI.pStages             = stages;
    pipeCI.pVertexInputState   = &viCI;
    pipeCI.pInputAssemblyState = &iaCI;
    pipeCI.pViewportState      = &vpCI;
    pipeCI.pRasterizationState = &rsCI;
    pipeCI.pMultisampleState   = &msCI;
    pipeCI.pColorBlendState    = &cbCI;
    pipeCI.layout              = pipelineLayout;
    pipeCI.renderPass          = renderPass;
    pipeCI.subpass             = 0;

    int64_t t0 = now_us();
    VkResult res = pfn_vkCreateGraphicsPipelines(device, cache, 1, &pipeCI, nullptr, out_pipeline);
    int64_t elapsed = now_us() - t0;

    pfn_vkDestroyShaderModule(device, fragMod, nullptr);
    pfn_vkDestroyShaderModule(device, vertMod, nullptr);

    return (res == VK_SUCCESS) ? elapsed : -1;
}

// ========== Path resolution ==========
static std::string get_project_root() {
    char exe_path[4096];
    uint32_t sz = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &sz) == 0) {
        char* real = realpath(exe_path, nullptr);
        if (real) {
            std::string p(real);
            free(real);
            while (p.size() > 1) {
                auto pos = p.rfind('/');
                if (pos == std::string::npos) break;
                p = p.substr(0, pos);
                std::string test = p + "/lib/shaders/spirv/s1_base_pbr.vert.spv";
                if (std::ifstream(test).good()) return p;
            }
        }
    }
    for (auto cand : {".", "BenchmarkApp", ".."}) {
        std::string test = std::string(cand) + "/lib/shaders/spirv/s1_base_pbr.vert.spv";
        if (std::ifstream(test).good()) return cand;
    }
    return ".";
}

// ========== Print helpers ==========
static std::string format_size(size_t bytes) {
    std::ostringstream ss;
    ss << (bytes + 512) / 1024 << " KB";
    return ss.str();
}

// UTF-8 box-drawing helpers
static const char* BOX_H  = "\xe2\x95\x90";  // ═
static const char* BOX_V  = "\xe2\x95\x91";  // ║
static const char* BOX_TL = "\xe2\x95\x94";  // ╔
static const char* BOX_TR = "\xe2\x95\x97";  // ╗
static const char* BOX_BL = "\xe2\x95\x9a";  // ╚
static const char* BOX_BR = "\xe2\x95\x9d";  // ╝
static const char* BOX_ML = "\xe2\x95\xa0";  // ╠
static const char* BOX_MR = "\xe2\x95\xa3";  // ╣
static const char* TBL_V  = "\xe2\x94\x82";  // │

static void print_hline(const char* left, const char* fill, const char* right, int width) {
    std::cout << left;
    for (int i = 0; i < width; i++) std::cout << fill;
    std::cout << right << "\n";
}

static void print_header_box(const std::string& lvp_name, const std::string& metal_name) {
    std::string dev_line = "Device: " + lvp_name;
    if (!metal_name.empty()) dev_line += " | " + metal_name;

    const int W = 67;
    // em dash is 3 bytes in UTF-8, so the visual title is shorter than .size()
    std::string title = "PSO Compilation \xe2\x80\x94 Mesa Lavapipe vs Metal";
    int title_visual_len = 44;  // actual visual character count

    auto center = [&](const std::string& s, int visual_len) -> std::string {
        int pad = (W - visual_len) / 2;
        std::string r = std::string(std::max(0, pad), ' ') + s;
        int fill = W - pad - visual_len;
        if (fill > 0) r += std::string(fill, ' ');
        return r;
    };

    print_hline(BOX_TL, BOX_H, BOX_TR, W);
    std::cout << BOX_V << center(title, title_visual_len) << BOX_V << "\n";
    print_hline(BOX_ML, BOX_H, BOX_MR, W);
    std::cout << BOX_V << center(dev_line, (int)dev_line.size()) << BOX_V << "\n";
    print_hline(BOX_BL, BOX_H, BOX_BR, W);
}

// ========== Main ==========
int main(int argc, char** argv) {
    std::string project_root;
    if (argc > 1) {
        project_root = argv[1];
    } else {
        project_root = get_project_root();
    }

    std::string spirv_dir = project_root + "/lib/shaders/spirv/";
    std::string lavapipe_path = project_root + "/third_party/mesa/builddir/src/gallium/targets/lavapipe/libvulkan_lvp.dylib";

    // Verify paths
    {
        std::string test = spirv_dir + "s1_base_pbr.vert.spv";
        if (!std::ifstream(test).good()) {
            std::cerr << "ERROR: Cannot find SPIR-V shaders at: " << spirv_dir << "\n";
            std::cerr << "Usage: " << argv[0] << " [project_root]\n";
            return 1;
        }
    }
    if (!std::ifstream(lavapipe_path).good()) {
        std::cerr << "ERROR: Cannot find Lavapipe at: " << lavapipe_path << "\n";
        return 1;
    }

    // ========== Load Vulkan ICD ==========
    if (!load_vulkan_icd(lavapipe_path.c_str())) {
        std::cerr << "FATAL: Cannot load Lavapipe ICD\n";
        return 1;
    }

    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "MacBenchmark";
    appInfo.apiVersion       = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instCI{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instCI.pApplicationInfo = &appInfo;

    VkInstance instance;
    if (pfn_vkCreateInstance(&instCI, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "FATAL: vkCreateInstance failed\n";
        return 1;
    }
    load_instance_funcs(instance);

    uint32_t gpuCount = 0;
    pfn_vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
    if (gpuCount == 0) {
        std::cerr << "FATAL: No Vulkan devices found\n";
        pfn_vkDestroyInstance(instance, nullptr);
        return 1;
    }
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    pfn_vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

    VkPhysicalDeviceProperties props;
    pfn_vkGetPhysicalDeviceProperties(gpus[0], &props);
    std::string lvp_device_name = props.deviceName;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo qCI{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qCI.queueFamilyIndex = 0;
    qCI.queueCount       = 1;
    qCI.pQueuePriorities = &priority;

    VkDeviceCreateInfo devCI{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    devCI.queueCreateInfoCount = 1;
    devCI.pQueueCreateInfos    = &qCI;

    VkDevice device;
    if (pfn_vkCreateDevice(gpus[0], &devCI, nullptr, &device) != VK_SUCCESS) {
        std::cerr << "FATAL: vkCreateDevice failed\n";
        pfn_vkDestroyInstance(instance, nullptr);
        return 1;
    }
    load_device_funcs(device);

    // ========== Create shared Vulkan objects ==========

    // Render pass (BGRA8, 1 color attachment)
    VkAttachmentDescription att{};
    att.format         = VK_FORMAT_B8G8R8A8_UNORM;
    att.samples        = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    att.finalLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &ref;

    VkRenderPassCreateInfo rpCI{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpCI.attachmentCount = 1;
    rpCI.pAttachments    = &att;
    rpCI.subpassCount    = 1;
    rpCI.pSubpasses      = &subpass;

    VkRenderPass renderPass;
    if (pfn_vkCreateRenderPass(device, &rpCI, nullptr, &renderPass) != VK_SUCCESS) {
        std::cerr << "FATAL: vkCreateRenderPass failed\n";
        pfn_vkDestroyDevice(device, nullptr);
        pfn_vkDestroyInstance(instance, nullptr);
        return 1;
    }

    // Descriptor set layouts matching the SPIR-V shader bindings:
    //   Set 0: binding 0 = SceneUBO (vert+frag), binding 1 = LightUBO (frag)
    //   Set 1: bindings 0..15 = combined image samplers (frag)
    VkDescriptorSetLayoutBinding set0Bindings[2]{};
    set0Bindings[0].binding         = 0;
    set0Bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    set0Bindings[0].descriptorCount = 1;
    set0Bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    set0Bindings[1].binding         = 1;
    set0Bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    set0Bindings[1].descriptorCount = 1;
    set0Bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo set0CI{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    set0CI.bindingCount = 2;
    set0CI.pBindings    = set0Bindings;

    VkDescriptorSetLayout set0Layout;
    pfn_vkCreateDescriptorSetLayout(device, &set0CI, nullptr, &set0Layout);

    VkDescriptorSetLayoutBinding set1Bindings[16]{};
    for (int i = 0; i < 16; i++) {
        set1Bindings[i].binding         = static_cast<uint32_t>(i);
        set1Bindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        set1Bindings[i].descriptorCount = 1;
        set1Bindings[i].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo set1CI{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    set1CI.bindingCount = 16;
    set1CI.pBindings    = set1Bindings;

    VkDescriptorSetLayout set1Layout;
    pfn_vkCreateDescriptorSetLayout(device, &set1CI, nullptr, &set1Layout);

    VkDescriptorSetLayout setLayouts[2] = { set0Layout, set1Layout };

    VkPipelineLayoutCreateInfo plCI{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    plCI.setLayoutCount = 2;
    plCI.pSetLayouts    = setLayouts;
    VkPipelineLayout pipelineLayout;
    pfn_vkCreatePipelineLayout(device, &plCI, nullptr, &pipelineLayout);

    // ========== Warmup with minimal shaders ==========
    {
        auto wv = read_file(spirv_dir + "minimal.vert.spv");
        auto wf = read_file(spirv_dir + "minimal.frag.spv");
        if (!wv.empty() && !wf.empty()) {
            VkPipeline warmup_pipe;
            auto t = create_pipeline_timed(device, wv, wf, renderPass, pipelineLayout, VK_NULL_HANDLE, &warmup_pipe);
            if (t >= 0) pfn_vkDestroyPipeline(device, warmup_pipe, nullptr);
        }
    }

    // ========== Benchmark each tier ==========
    constexpr int kWarmRuns = 3;
    std::vector<TierResult> results;

    for (int ti = 0; ti < kNumTiers; ti++) {
        const auto& tier = kTiers[ti];
        TierResult tr;
        tr.name = tier.display_name;

        auto vert_data = read_file(spirv_dir + tier.vert_filename);
        auto frag_data = read_file(spirv_dir + tier.frag_filename);
        if (vert_data.empty() || frag_data.empty()) {
            std::cerr << "  WARNING: Cannot read " << tier.name << " SPIR-V files\n";
            tr.ok = false;
            results.push_back(tr);
            continue;
        }
        tr.spv_bytes = vert_data.size() + frag_data.size();

        // 1 warmup run (discard)
        {
            VkPipeline p;
            auto t = create_pipeline_timed(device, vert_data, frag_data, renderPass, pipelineLayout, VK_NULL_HANDLE, &p);
            if (t >= 0) pfn_vkDestroyPipeline(device, p, nullptr);
        }

        // 1 cold run (no pipeline cache)
        {
            VkPipeline p;
            auto t = create_pipeline_timed(device, vert_data, frag_data, renderPass, pipelineLayout, VK_NULL_HANDLE, &p);
            if (t < 0) {
                tr.ok = false;
                results.push_back(tr);
                continue;
            }
            tr.cold_us = t;
            pfn_vkDestroyPipeline(device, p, nullptr);
        }

        // 3 warm runs (reuse pipeline cache)
        VkPipelineCacheCreateInfo cacheCI{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
        VkPipelineCache cache;
        pfn_vkCreatePipelineCache(device, &cacheCI, nullptr, &cache);

        // Prime the cache
        {
            VkPipeline p;
            auto t = create_pipeline_timed(device, vert_data, frag_data, renderPass, pipelineLayout, cache, &p);
            if (t >= 0) pfn_vkDestroyPipeline(device, p, nullptr);
        }

        int64_t warm_sum = 0;
        int warm_count = 0;
        for (int r = 0; r < kWarmRuns; r++) {
            VkPipeline p;
            auto t = create_pipeline_timed(device, vert_data, frag_data, renderPass, pipelineLayout, cache, &p);
            if (t >= 0) {
                warm_sum += t;
                warm_count++;
                pfn_vkDestroyPipeline(device, p, nullptr);
            }
        }

        pfn_vkDestroyPipelineCache(device, cache, nullptr);

        if (warm_count > 0) {
            tr.warm_us = warm_sum / warm_count;
        }
        tr.speedup = (tr.warm_us > 0) ? (double)tr.cold_us / (double)tr.warm_us : 0.0;
        tr.ok = true;
        results.push_back(tr);
    }

    // ========== Run Metal benchmark ==========
    std::vector<MetalTierResult> metal_results;
    std::string metal_device_name;
    try {
        metal_results = run_metal_all_tiers();
    } catch (...) {
        // Metal not available
    }

    // ========== Print results ==========
    std::cout << "\n";
    print_header_box(lvp_device_name, metal_device_name);

    // Vulkan table
    const int TW = 63;
    std::cout << "\nMesa Lavapipe (Vulkan) \xe2\x80\x94 vkCreateGraphicsPipelines (\xc2\xb5s)\n";
    print_hline("", BOX_H, "", TW);

    printf(" %-22s%s %7s %s %6s %s %8s %s %8s %s\n",
           "Workload", TBL_V, "Cold", TBL_V, "Warm", TBL_V, "Speedup", TBL_V, "SPV Size", TBL_V);

    print_hline("", BOX_H, "", TW);

    for (auto& tr : results) {
        if (!tr.ok) {
            printf(" %-22s%s %7s %s %6s %s %8s %s %8s %s\n",
                   tr.name.c_str(), TBL_V, "FAIL", TBL_V, "FAIL", TBL_V, "---", TBL_V, "---", TBL_V);
            continue;
        }
        char speedup_buf[16];
        snprintf(speedup_buf, sizeof(speedup_buf), "%.1fx", tr.speedup);
        printf(" %-22s%s %5lld   %s %5lld  %s %6s   %s %6s   %s\n",
               tr.name.c_str(), TBL_V,
               (long long)tr.cold_us, TBL_V,
               (long long)tr.warm_us, TBL_V,
               speedup_buf, TBL_V,
               format_size(tr.spv_bytes).c_str(), TBL_V);
    }

    print_hline("", BOX_H, "", TW);

    // Metal table
    if (!metal_results.empty()) {
        const int MW = 53;
        std::cout << "\nMetal (Native) \xe2\x80\x94 newRenderPipelineState (\xc2\xb5s)\n";
        print_hline("", BOX_H, "", MW);
        printf(" %-22s%s %9s %s %9s %s %9s %s\n",
               "Workload", TBL_V, "Compile", TBL_V, "PSO", TBL_V, "Total", TBL_V);
        print_hline("", BOX_H, "", MW);

        for (auto& mr : metal_results) {
            if (!mr.ok) {
                printf(" %-22s%s %9s %s %9s %s %9s %s\n",
                       mr.tier.c_str(), TBL_V, "FAIL", TBL_V, "FAIL", TBL_V, "FAIL", TBL_V);
                continue;
            }
            printf(" %-22s%s %7lld   %s %7lld   %s %7lld   %s\n",
                   mr.tier.c_str(), TBL_V,
                   (long long)mr.compile_us, TBL_V,
                   (long long)mr.pipeline_us, TBL_V,
                   (long long)mr.total_us, TBL_V);
        }

        print_hline("", BOX_H, "", MW);
    }

    std::cout << "\nDone.\n";

    // Cleanup
    pfn_vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    pfn_vkDestroyDescriptorSetLayout(device, set1Layout, nullptr);
    pfn_vkDestroyDescriptorSetLayout(device, set0Layout, nullptr);
    pfn_vkDestroyRenderPass(device, renderPass, nullptr);
    pfn_vkDestroyDevice(device, nullptr);
    pfn_vkDestroyInstance(instance, nullptr);

    return 0;
}
