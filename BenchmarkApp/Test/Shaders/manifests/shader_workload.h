#pragma once

#include <string>

namespace benchmark {

struct ShaderWorkload {
    std::string tier;
    int texture_sample_count = 0;
    int static_switch_count = 0;
    int permutation_count = 0;
};

ShaderWorkload make_base_pbr_workload();
ShaderWorkload make_permutation_stress_workload();

}  // namespace benchmark
