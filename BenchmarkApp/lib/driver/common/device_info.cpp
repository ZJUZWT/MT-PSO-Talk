#include "driver/common/apple_device_lookup.h"
#include "driver/common/device_info.h"

#include <string>

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#include <fstream>
#include <sstream>
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#endif

namespace benchmark {

#if defined(__APPLE__)
namespace {

std::string read_sysctl_string(const char* name) {
    size_t size = 0;
    if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0) {
        return {};
    }

    std::string value(size, '\0');
    if (sysctlbyname(name, value.data(), &size, nullptr, 0) != 0 || size == 0) {
        return {};
    }

    value.resize(size);
    while (!value.empty() && (value.back() == '\0' || value.back() == '\n')) {
        value.pop_back();
    }
    return value;
}

std::string read_uname_field_machine() {
    utsname system_info{};
    if (uname(&system_info) != 0) {
        return {};
    }
    return std::string(system_info.machine);
}

std::string read_uname_field_release() {
    utsname system_info{};
    if (uname(&system_info) != 0) {
        return {};
    }
    return std::string(system_info.release);
}

std::string build_os_version_label(const char* platform_label) {
    std::string version = read_sysctl_string("kern.osproductversion");
    if (version.empty()) {
        version = read_uname_field_release();
    }
    if (version.empty()) {
        return std::string(platform_label);
    }
    return std::string(platform_label) + " " + version;
}

std::string format_apple_device_model(const std::string& identifier, const AppleProductInfo& info) {
    if (identifier.empty()) {
        return info.marketing_name;
    }
    if (info.marketing_name.empty()) {
        return identifier;
    }
    return info.marketing_name + " [" + identifier + "]";
}

}  // namespace
#endif

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
    std::string identifier = read_sysctl_string("hw.machine");
    if (identifier.empty()) {
        identifier = read_uname_field_machine();
    }

#if TARGET_OS_SIMULATOR
    if (identifier.empty()) {
        info.device_model = "iOS Simulator";
    } else {
        info.device_model = "iOS Simulator [" + identifier + "]";
    }

    info.soc = read_sysctl_string("machdep.cpu.brand_string");
    if (info.soc.empty()) {
        info.soc = identifier.empty() ? "Simulator CPU" : identifier;
    }
    info.os_version = build_os_version_label("iOS Simulator");
#else
    const AppleProductInfo product_info = lookup_apple_product_info(identifier);
    info.device_model = format_apple_device_model(identifier, product_info);
    if (info.device_model.empty()) {
        info.device_model = "iOS Device";
    }

    info.soc = product_info.soc;
    if (info.soc.empty()) {
        std::string hardware_model = read_sysctl_string("hw.model");
        if (!hardware_model.empty()) {
            info.soc = "Unknown Apple SoC [" + hardware_model + "]";
        } else if (!identifier.empty()) {
            info.soc = "Unknown Apple SoC [" + identifier + "]";
        } else {
            info.soc = "Unknown Apple SoC";
        }
    }
    info.os_version = build_os_version_label("iOS");
#endif
    info.gpu_name = "Apple GPU";
    info.driver_version = "N/A";
    return info;
}

#elif defined(_WIN32)

#include <windows.h>
#include <intrin.h>

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = "Windows PC";
    info.gpu_name = "Unknown";
    info.driver_version = "N/A";
    
    // Get Windows version
    OSVERSIONINFOEXA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
    #pragma warning(push)
    #pragma warning(disable: 4996)
    if (GetVersionExA((OSVERSIONINFOA*)&osvi)) {
        info.os_version = "Windows " + std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
    } else {
        info.os_version = "Windows";
    }
    #pragma warning(pop)
    
    // Get CPU brand string using CPUID
    int cpuInfo[4] = {-1};
    char cpuBrandString[0x40] = {0};
    
    // Check if CPUID supports brand string
    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExIds = cpuInfo[0];
    
    if (nExIds >= 0x80000004) {
        // Get brand string (48 bytes across 3 CPUID calls)
        __cpuid(cpuInfo, 0x80000002);
        memcpy(cpuBrandString, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000003);
        memcpy(cpuBrandString + 16, cpuInfo, sizeof(cpuInfo));
        __cpuid(cpuInfo, 0x80000004);
        memcpy(cpuBrandString + 32, cpuInfo, sizeof(cpuInfo));
        
        // Trim whitespace
        std::string brand(cpuBrandString);
        size_t start = brand.find_first_not_of(" \t\r\n");
        size_t end = brand.find_last_not_of(" \t\r\n");
        if (start != std::string::npos && end != std::string::npos) {
            info.soc = brand.substr(start, end - start + 1);
        } else {
            info.soc = brand;
        }
    } else {
        info.soc = "Unknown CPU";
    }
    
    // Try to get computer name as device model
    char computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(computerName);
    if (GetComputerNameA(computerName, &size)) {
        info.device_model = std::string(computerName);
    }
    
    return info;
}

#elif defined(__APPLE__)

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = read_sysctl_string("hw.model");
    if (info.device_model.empty()) {
        info.device_model = read_uname_field_machine();
    }
    if (info.device_model.empty()) {
        info.device_model = "Mac";
    }

    info.soc = read_sysctl_string("machdep.cpu.brand_string");
    if (info.soc.empty()) {
        info.soc = "Unknown Apple CPU";
    }

    info.os_version = build_os_version_label("macOS");
    info.gpu_name = "Apple GPU";
    info.driver_version = "N/A";
    return info;
}

#elif defined(__linux__)

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = "Linux Device";
    info.soc = "Unknown";
    info.os_version = "Linux";
    info.gpu_name = "Unknown";
    info.driver_version = "N/A";
    return info;
}

#else

DeviceInfo query_device_info() {
    DeviceInfo info;
    info.device_model = "Desktop Device";
    info.soc = "Unknown";
    info.os_version = "Unknown OS";
    info.gpu_name = "Unknown";
    info.driver_version = "N/A";
    return info;
}

#endif

}  // namespace benchmark
