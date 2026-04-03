#pragma once

#include <string>
#include <vector>

#include "results/schema/result_types.h"

namespace benchmark {

struct GraphicsBenchmarkConfig {
    int iterations_per_case = 3;
    bool run_cold_cache = true;
    bool run_warm_cache = true;
    bool use_real_backends = false;  // Try real GPU contexts; fall back to simulation
};

class GraphicsBenchmark {
public:
    // Run all backends x all workload tiers
    std::vector<GraphicsResult> run_all(
        const GraphicsBenchmarkConfig& config = {});

    // Run a specific backend with all workload tiers
    std::vector<GraphicsResult> run_backend(
        const std::string& mode,
        const GraphicsBenchmarkConfig& config = {});
};

}  // namespace benchmark
