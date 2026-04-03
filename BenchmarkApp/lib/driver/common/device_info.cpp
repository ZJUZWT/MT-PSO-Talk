#include "driver/common/device_info.h"

#include <string>

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#include <fstream>
#include <sstream>
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace benchmark {

#if defined(__ANDROID__)

static std::string get_android_prop(const char* name) {
    char value[PROP_VALUE_MAX] = {};
    __system_property_get(name, value);
    return std::string(value);
}

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = get_android_prop("ro.product.model");
    if (info.device_model.empty()) info.device_model = "Android Device";

    info.soc = get_android_prop("ro.hardware.chipname");
    if (info.soc.empty()) {
        info.soc = get_android_prop("ro.board.platform");
    }
    if (info.soc.empty()) info.soc = "unknown";

    std::string sdk = get_android_prop("ro.build.version.sdk");
    std::string release = get_android_prop("ro.build.version.release");
    info.os_version = "Android " + release + " (API " + sdk + ")";

    // GPU name is best obtained via Vulkan/GL at runtime; provide placeholder
    info.gpu_name = "unknown";
    info.driver_version = "unknown";
    return info;
}

#elif defined(__APPLE__) && TARGET_OS_IPHONE

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = "iPhone";
    info.soc = "Apple Silicon";
    info.os_version = "iOS";
    info.gpu_name = "Apple GPU";
    info.driver_version = "N/A";
    return info;
}

#else

// Mac / desktop stub
DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = "Mac";
    info.soc = "Apple Silicon";
    info.os_version = "macOS";
    info.gpu_name = "Simulated";
    info.driver_version = "N/A";
    return info;
}

#endif

}  // namespace benchmark
