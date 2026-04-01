#include "Shaders/manifests/shader_workload.h"
#include "TestSupport/assert.h"

int main() {
    const auto workload = benchmark::make_base_pbr_workload();
    test_support::expect_true(workload.tier == "S1_BasePBR", "tier");
    test_support::expect_true(workload.permutation_count > 0, "permutations");
    return test_support::finish();
}
