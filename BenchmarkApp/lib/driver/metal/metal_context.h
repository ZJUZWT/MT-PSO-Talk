#pragma once

#include <string>

namespace benchmark {

class MetalContext {
public:
    MetalContext() = default;
    ~MetalContext();

    bool init();
    void destroy();
    bool is_available() const;
    std::string device_name() const;

    // Opaque pointer to id<MTLDevice> — only valid on Apple platforms
    void* mtl_device() const;

private:
    bool available_ = false;
    std::string device_name_ = "Unavailable";
    void* device_ptr_ = nullptr;
};

}  // namespace benchmark
