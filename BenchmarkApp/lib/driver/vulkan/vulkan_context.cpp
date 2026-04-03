#include "driver/vulkan/vulkan_context.h"

#include <cstring>

namespace benchmark {

#if defined(__ANDROID__) || defined(HAVE_VULKAN)

bool VulkanContext::init() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "PSO Benchmark";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "BenchmarkEngine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_ci{};
    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pApplicationInfo = &app_info;

    if (vkCreateInstance(&instance_ci, nullptr, &instance_) != VK_SUCCESS) {
        return false;
    }

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    if (device_count == 0) {
        destroy();
        return false;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());
    physical_device_ = devices[0];

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(physical_device_, &props);
    device_name_ = props.deviceName;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queue_family_count, queue_families.data());

    bool found_queue = false;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_index_ = i;
            found_queue = true;
            break;
        }
    }
    if (!found_queue) {
        destroy();
        return false;
    }

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_ci{};
    queue_ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_ci.queueFamilyIndex = queue_family_index_;
    queue_ci.queueCount = 1;
    queue_ci.pQueuePriorities = &queue_priority;

    VkDeviceCreateInfo device_ci{};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = &queue_ci;

    if (vkCreateDevice(physical_device_, &device_ci, nullptr, &device_) != VK_SUCCESS) {
        destroy();
        return false;
    }

    available_ = true;
    return true;
}

void VulkanContext::destroy() {
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
    available_ = false;
}

VkPipelineCache VulkanContext::create_pipeline_cache() const {
    VkPipelineCacheCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkPipelineCache cache = VK_NULL_HANDLE;
    vkCreatePipelineCache(device_, &ci, nullptr, &cache);
    return cache;
}

#else

// Stub — Vulkan not available
bool VulkanContext::init() {
    available_ = false;
    return false;
}

void VulkanContext::destroy() {
    available_ = false;
}

#endif

VulkanContext::~VulkanContext() {
    destroy();
}

std::string VulkanContext::device_name() const {
    return device_name_;
}

bool VulkanContext::is_available() const {
    return available_;
}

}  // namespace benchmark
