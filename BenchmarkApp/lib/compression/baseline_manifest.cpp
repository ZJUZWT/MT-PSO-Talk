#include "compression/baseline_manifest.h"

namespace benchmark {

CompressionBaselineManifest make_default_zstd_baseline() {
    return CompressionBaselineManifest{
        "zstd",
        "1.5.5",
        "3",
        "pso-cache-zstd-v1",
        "a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1f3a5e7c9b0d2f4",
        "e7c9b0d2f4a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1f3a5",
        262144,
    };
}

CompressionBaselineManifest make_lz4_baseline() {
    return CompressionBaselineManifest{
        "lz4",
        "1.9.4",
        "1",
        "pso-cache-lz4-v1",
        "b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1f3a5e7c9b0d2f4a3f2c8e1",
        "d1f3a5e7c9b0d2f4a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9",
        262144,
    };
}

CompressionBaselineManifest make_zlib_baseline() {
    return CompressionBaselineManifest{
        "zlib",
        "1.3",
        "6",
        "pso-cache-zlib-v1",
        "c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1f3a5e7c9b0d2f4a3f2",
        "f3a5e7c9b0d2f4a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1",
        262144,
    };
}

CompressionBaselineManifest make_snappy_baseline() {
    return CompressionBaselineManifest{
        "snappy",
        "1.1.10",
        "default",
        "pso-cache-snappy-v1",
        "d0f1a3e5c7b9d1f3a5e7c9b0d2f4a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9",
        "a5e7c9b0d2f4a3f2c8e1b4d7f6a09e5c3b1d8f2a4e6c7b9d0f1a3e5c7b9d1f3",
        262144,
    };
}

std::vector<CompressionBaselineManifest> make_all_compression_baselines() {
    return {
        make_default_zstd_baseline(),
        make_lz4_baseline(),
        make_zlib_baseline(),
        make_snappy_baseline(),
    };
}

}  // namespace benchmark
