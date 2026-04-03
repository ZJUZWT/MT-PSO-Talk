#pragma once

#include <cstdint>
#include <string>

namespace benchmark {

struct MetalCompileResult {
    int64_t create_library_us = 0;
    int64_t create_functions_us = 0;
    int64_t create_pipeline_us = 0;
    int64_t total_us = 0;
    std::string status = "passed";
};

// device is an opaque pointer to id<MTLDevice>
MetalCompileResult compile_metal_pipeline(void* device,
                                          const std::string& msl_source);

}  // namespace benchmark
