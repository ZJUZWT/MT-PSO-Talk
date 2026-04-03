#pragma once

#include <string>
#include <vector>

#if defined(__ANDROID__) || defined(HAVE_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace benchmark {

class VulkanContext {
public:
    VulkanContext() = default;
    ~VulkanContext();

    bool init();
    void destroy();

    std::string device_name() const;
    bool is_available() const;

#if defined(__ANDROID__) || defined(HAVE_VULKAN)
    VkDevice device() const { return device_; }
    VkPhysicalDevice physical_device() const { return physical_device_; }
    VkPipelineCache create_pipeline_cache() const;
#endif

private:
    bool available_ = false;
    std::string device_name_ = "Unavailable";

#if defined(__ANDROID__) || defined(HAVE_VULKAN)
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    uint32_t queue_family_index_ = 0;
#endif
};

}  // namespace benchmark
