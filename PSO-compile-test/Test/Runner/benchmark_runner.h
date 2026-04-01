#pragma once

#include <string>
#include <vector>

#include "Runner/benchmark_case.h"

namespace benchmark {

class BenchmarkRunner {
public:
    std::vector<BenchmarkCase> plan_for_mode(const std::string& mode) const;
};

}  // namespace benchmark
