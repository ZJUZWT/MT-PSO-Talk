#pragma once

#include <string>
#include <vector>

#include "runner/benchmark_case.h"

namespace benchmark {

class BenchmarkRunner {
public:
    std::vector<BenchmarkCase> plan_for_mode(const std::string& mode) const;
    std::vector<BenchmarkCase> plan_all() const;
    static std::vector<std::string> all_modes();
};

}  // namespace benchmark
