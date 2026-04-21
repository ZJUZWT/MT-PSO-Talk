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
    std::string device_model;
    std::string soc;
    std::string os_version;
    std::string mesa_version;
    int iteration_index = 0;
    bool cold_cache = true;
    std::string status = "passed";
    std::int64_t total_us = 0;
    std::int64_t shader_compile_us = 0;
    std::int64_t link_pipeline_create_us = 0;
    std::int64_t first_draw_ready_us = 0;
    std::vector<StageTiming> stage_breakdown;
};

struct CompressionResult {
    std::string platform;
    std::string algorithm;
    std::string baseline_package_id;
    std::string device_model;
    std::string soc;
    std::string os_version;
    std::string version;
    std::string level;
    std::string payload_profile = "custom";
    int iteration_index = 0;
    std::int64_t input_size = 0;
    std::int64_t compressed_size = 0;
    double compression_ratio = 0.0;
    std::int64_t compress_us = 0;
    double throughput_mbps = 0.0;
    std::string input_hash;
    std::string compressed_output_hash;
    std::int64_t decompress_us = 0;
    std::string decompressed_output_hash;
    bool roundtrip_hash_match = false;
    bool roundtrip_byte_match = false;
    std::string status = "passed";
};

}  // namespace benchmark
