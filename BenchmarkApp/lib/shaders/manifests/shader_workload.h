#pragma once

#include <string>
#include <vector>

namespace benchmark {

struct ShaderWorkload {
    std::string tier;
    int texture_sample_count = 0;
    int static_switch_count = 0;
    int permutation_count = 0;
    int instruction_count = 0;       // approx instruction scale
    int control_flow_depth = 0;      // nesting depth
    int resource_binding_count = 0;  // textures + buffers + uniforms
    int stage_count = 1;             // vertex + fragment = 2, etc
};

ShaderWorkload make_base_pbr_workload();
ShaderWorkload make_material_layered_workload();    // S2
ShaderWorkload make_feature_heavy_workload();       // S3
ShaderWorkload make_permutation_stress_workload();
std::vector<ShaderWorkload> make_all_workloads();   // returns all 4 tiers

}  // namespace benchmark
