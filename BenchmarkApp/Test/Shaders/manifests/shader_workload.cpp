#include "Shaders/manifests/shader_workload.h"

namespace benchmark {

ShaderWorkload make_base_pbr_workload() {
    return ShaderWorkload{
        "S1_BasePBR",
        6,
        2,
        8,
    };
}

ShaderWorkload make_permutation_stress_workload() {
    return ShaderWorkload{
        "S4_PermutationStress",
        16,
        8,
        64,
    };
}

}  // namespace benchmark
