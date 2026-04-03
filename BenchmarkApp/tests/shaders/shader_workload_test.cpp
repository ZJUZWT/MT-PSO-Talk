#include "shaders/manifests/shader_workload.h"
#include "test_support/assert.h"

#include <set>
#include <string>

int main() {
    // S1
    const auto s1 = benchmark::make_base_pbr_workload();
    test_support::expect_equal(s1.tier, std::string("S1_BasePBR"), "S1 tier");
    test_support::expect_greater_than(s1.permutation_count, 0, "S1 permutations > 0");
    test_support::expect_equal(s1.instruction_count, 200, "S1 instructions");
    test_support::expect_equal(s1.stage_count, 2, "S1 stages");
    test_support::expect_greater_than(s1.texture_sample_count, 0, "S1 texture_sample_count > 0");
    test_support::expect_greater_than(s1.control_flow_depth, 0, "S1 control_flow_depth > 0");
    test_support::expect_greater_than(s1.resource_binding_count, 0, "S1 resource_binding_count > 0");

    // S2
    const auto s2 = benchmark::make_material_layered_workload();
    test_support::expect_equal(s2.tier, std::string("S2_MaterialLayered"), "S2 tier");
    test_support::expect_equal(s2.texture_sample_count, 12, "S2 textures");
    test_support::expect_equal(s2.instruction_count, 450, "S2 instructions");
    test_support::expect_equal(s2.control_flow_depth, 3, "S2 depth");
    test_support::expect_equal(s2.resource_binding_count, 14, "S2 bindings");
    test_support::expect_greater_than(s2.permutation_count, 0, "S2 permutations > 0");
    test_support::expect_greater_than(s2.stage_count, 0, "S2 stage_count > 0");

    // S3
    const auto s3 = benchmark::make_feature_heavy_workload();
    test_support::expect_equal(s3.tier, std::string("S3_FeatureHeavy"), "S3 tier");
    test_support::expect_equal(s3.texture_sample_count, 16, "S3 textures");
    test_support::expect_equal(s3.instruction_count, 800, "S3 instructions");
    test_support::expect_equal(s3.control_flow_depth, 5, "S3 depth");
    test_support::expect_equal(s3.stage_count, 3, "S3 stages");
    test_support::expect_greater_than(s3.permutation_count, 0, "S3 permutations > 0");
    test_support::expect_greater_than(s3.resource_binding_count, 0, "S3 resource_binding_count > 0");

    // S4
    const auto s4 = benchmark::make_permutation_stress_workload();
    test_support::expect_equal(s4.tier, std::string("S4_PermutationStress"), "S4 tier");
    test_support::expect_equal(s4.permutation_count, 64, "S4 permutations");
    test_support::expect_equal(s4.instruction_count, 350, "S4 instructions");
    test_support::expect_greater_than(s4.texture_sample_count, 0, "S4 texture_sample_count > 0");
    test_support::expect_greater_than(s4.control_flow_depth, 0, "S4 control_flow_depth > 0");
    test_support::expect_greater_than(s4.resource_binding_count, 0, "S4 resource_binding_count > 0");
    test_support::expect_greater_than(s4.stage_count, 0, "S4 stage_count > 0");

    // Verify S3 > S1 in complexity metrics
    test_support::expect_greater_than(s3.instruction_count, s1.instruction_count,
        "S3 instructions > S1");
    test_support::expect_greater_than(s3.texture_sample_count, s1.texture_sample_count,
        "S3 textures > S1");
    test_support::expect_greater_than(s3.control_flow_depth, s1.control_flow_depth,
        "S3 depth > S1");

    // make_all_workloads returns exactly 4 with unique tiers
    const auto all = benchmark::make_all_workloads();
    test_support::expect_equal(all.size(), static_cast<size_t>(4), "all workloads count");
    test_support::expect_equal(all[0].tier, std::string("S1_BasePBR"), "all[0] tier");
    test_support::expect_equal(all[1].tier, std::string("S2_MaterialLayered"), "all[1] tier");
    test_support::expect_equal(all[2].tier, std::string("S3_FeatureHeavy"), "all[2] tier");
    test_support::expect_equal(all[3].tier, std::string("S4_PermutationStress"), "all[3] tier");

    // Verify all tiers are unique
    std::set<std::string> tiers;
    for (const auto& w : all) {
        tiers.insert(w.tier);
    }
    test_support::expect_equal(tiers.size(), static_cast<size_t>(4), "all 4 tiers unique");

    return test_support::finish();
}
