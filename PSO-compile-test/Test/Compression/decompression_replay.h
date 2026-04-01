#pragma once

#include "Compression/baseline_manifest.h"
#include "Results/schema/result_types.h"

namespace benchmark {

CompressionResult simulate_replay(
    const CompressionBaselineManifest& baseline,
    int payload_size);

}  // namespace benchmark
