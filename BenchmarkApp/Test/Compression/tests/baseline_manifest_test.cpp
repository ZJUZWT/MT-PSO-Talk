#include "Compression/baseline_manifest.h"
#include "TestSupport/assert.h"

int main() {
    const auto baseline = benchmark::make_default_zstd_baseline();
    test_support::expect_true(baseline.algorithm == "zstd", "algorithm");
    test_support::expect_true(!baseline.baseline_package_id.empty(), "baseline package id");
    return test_support::finish();
}
