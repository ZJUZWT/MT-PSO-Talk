#include "shaders/manifests/shader_workload.h"

namespace benchmark {

ShaderWorkload make_base_pbr_workload() {
    return ShaderWorkload{
        "S1_BasePBR",
        6,   // texture_sample_count
        2,   // static_switch_count
        8,   // permutation_count
        200, // instruction_count
        2,   // control_flow_depth
        8,   // resource_binding_count
        2,   // stage_count
    };
}

ShaderWorkload make_material_layered_workload() {
    return ShaderWorkload{
        "S2_MaterialLayered",
        12,  // texture_sample_count
        4,   // static_switch_count
        24,  // permutation_count
        450, // instruction_count
        3,   // control_flow_depth
        14,  // resource_binding_count
        2,   // stage_count
    };
}

ShaderWorkload make_feature_heavy_workload() {
    return ShaderWorkload{
        "S3_FeatureHeavy",
        16,  // texture_sample_count
        8,   // static_switch_count
        48,  // permutation_count
        800, // instruction_count
        5,   // control_flow_depth
        20,  // resource_binding_count
        3,   // stage_count (vertex + geometry + fragment)
    };
}

ShaderWorkload make_permutation_stress_workload() {
    return ShaderWorkload{
        "S4_PermutationStress",
        16,  // texture_sample_count
        8,   // static_switch_count
        64,  // permutation_count
        350, // instruction_count
        3,   // control_flow_depth
        16,  // resource_binding_count
        2,   // stage_count
    };
}

std::vector<ShaderWorkload> make_all_workloads() {
    return {
        make_base_pbr_workload(),
        make_material_layered_workload(),
        make_feature_heavy_workload(),
        make_permutation_stress_workload(),
    };
}

}  // namespace benchmark
