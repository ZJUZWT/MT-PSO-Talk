#pragma once

#include <string>

namespace benchmark {

struct BenchmarkCase {
    std::string mode;
    std::string api;
    std::string driver_mode;
    std::string workload_tier;
};

}  // namespace benchmark
