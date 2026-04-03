#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace benchmark {

struct CompressionBaselineManifest {
    std::string algorithm;
    std::string version;
    std::string level;
    std::string baseline_package_id;
    std::string compressed_hash;
    std::string decompressed_hash;
    int64_t input_size = 0;
};

CompressionBaselineManifest make_default_zstd_baseline();
CompressionBaselineManifest make_lz4_baseline();
CompressionBaselineManifest make_zlib_baseline();
CompressionBaselineManifest make_snappy_baseline();
std::vector<CompressionBaselineManifest> make_all_compression_baselines();

}  // namespace benchmark
