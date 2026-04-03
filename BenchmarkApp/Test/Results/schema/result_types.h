#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace benchmark {

struct StageTiming {
    std::string name;
    std::int64_t duration_us = 0;
};

struct GraphicsResult {
    std::string platform;
    std::string api;
    std::string driver_mode;
    std::string case_name;
    std::int64_t total_us = 0;
    std::vector<StageTiming> stage_breakdown;
};

struct CompressionResult {
    std::string platform;
    std::string algorithm;
    std::string baseline_package_id;
    std::int64_t decompress_us = 0;
    std::string decompressed_output_hash;
};

}  // namespace benchmark
