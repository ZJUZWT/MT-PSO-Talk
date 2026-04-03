#include "Compression/compression_job.h"

namespace benchmark {

CompressionJob make_replay_job(
    const CompressionBaselineManifest& baseline,
    int payload_size) {
    return CompressionJob{baseline, payload_size};
}

}  // namespace benchmark
