#include "Compression/baseline_manifest.h"
#include "Compression/decompression_replay.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::CompressionBaselineManifest baseline =
        benchmark::make_default_zstd_baseline();
    const auto result = benchmark::simulate_replay(baseline, 4096);

    test_support::expect_true(
        result.baseline_package_id == baseline.baseline_package_id,
        "baseline linked");
    test_support::expect_true(result.decompress_us >= 0, "timing emitted");

    return test_support::finish();
}
