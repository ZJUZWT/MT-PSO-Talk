#pragma once

#include <string>

namespace benchmark {

struct ShaderSource {
    std::string glsl_vert;
    std::string glsl_frag;
    std::string metal;
};

/// Load embedded shader source for a given workload tier.
/// @param tier One of: "S1_BasePBR", "S2_MaterialLayered",
///             "S3_FeatureHeavy", "S4_PermutationStress"
/// @return ShaderSource with GLSL vertex, GLSL fragment, and MSL source.
///         Returns empty strings if the tier is unknown.
ShaderSource load_shader_source(const std::string& tier);

} // namespace benchmark
