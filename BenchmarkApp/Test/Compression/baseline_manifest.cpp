#include "Compression/baseline_manifest.h"

namespace benchmark {

CompressionBaselineManifest make_default_zstd_baseline() {
    return CompressionBaselineManifest{
        "zstd",
        "default",
        "zstd-default-sample",
        "compressed-placeholder-hash",
        "decompressed-placeholder-hash",
    };
}

}  // namespace benchmark
