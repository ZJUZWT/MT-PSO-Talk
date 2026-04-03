#pragma once

#include <string>

namespace benchmark {

struct CompressionBaselineManifest {
    std::string algorithm;
    std::string version;
    std::string baseline_package_id;
    std::string compressed_hash;
    std::string decompressed_hash;
};

CompressionBaselineManifest make_default_zstd_baseline();

}  // namespace benchmark
