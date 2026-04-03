#pragma once

#include <cstdint>
#include <vector>

namespace benchmark {

// Generate synthetic PSO cache data of given size.
// Uses patterns typical of pipeline state caches:
// header + repeated shader hash blocks + state vectors with padding.
std::vector<uint8_t> generate_pso_cache_payload(int64_t target_size);

// Generate with specific compressibility ratio.
// 0.0 = random (incompressible), 1.0 = highly compressible (mostly zeros/repeats).
std::vector<uint8_t> generate_payload_with_compressibility(int64_t target_size, double compressibility);

}  // namespace benchmark
