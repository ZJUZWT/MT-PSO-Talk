#pragma once

#include <string>

#include "Compression/baseline_manifest.h"

namespace benchmark {

struct CompressionJob {
    CompressionBaselineManifest baseline;
    int payload_size = 0;
};

CompressionJob make_replay_job(
    const CompressionBaselineManifest& baseline,
    int payload_size);

}  // namespace benchmark
