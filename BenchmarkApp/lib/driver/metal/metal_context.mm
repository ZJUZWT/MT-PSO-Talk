#include "driver/metal/metal_context.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__APPLE__) && TARGET_OS_IPHONE
#import <Metal/Metal.h>
#endif

namespace benchmark {

#if defined(__APPLE__) && TARGET_OS_IPHONE

bool MetalContext::init() {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        available_ = false;
        return false;
    }
    device_ptr_ = (__bridge_retained void*)device;
    device_name_ = [[device name] UTF8String];
    available_ = true;
    return true;
}

void MetalContext::destroy() {
    if (device_ptr_) {
        CFRelease(device_ptr_);
        device_ptr_ = nullptr;
    }
    available_ = false;
}

#else

// Stub for Mac desktop / non-iOS
bool MetalContext::init() {
    available_ = false;
    return false;
}

void MetalContext::destroy() {
    available_ = false;
}

#endif

MetalContext::~MetalContext() {
    destroy();
}

bool MetalContext::is_available() const {
    return available_;
}

std::string MetalContext::device_name() const {
    return device_name_;
}

void* MetalContext::mtl_device() const {
    return device_ptr_;
}

}  // namespace benchmark
