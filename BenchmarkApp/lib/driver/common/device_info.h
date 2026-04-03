#pragma once

#include <string>

namespace benchmark {

struct DeviceInfo {
    std::string device_model;    // e.g. "Pixel 8 Pro"
    std::string soc;             // e.g. "Tensor G3"
    std::string os_version;      // e.g. "Android 14"
    std::string gpu_name;        // e.g. "Adreno 750"
    std::string driver_version;  // e.g. "Mesa 24.1" or "v615.0"
};

DeviceInfo query_device_info();

}  // namespace benchmark
