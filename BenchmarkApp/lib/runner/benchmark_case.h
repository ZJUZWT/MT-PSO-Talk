#pragma once

#include <string>

namespace benchmark {

struct BenchmarkCase {
    std::string mode;
    std::string api;
    std::string driver_mode;
    std::string workload_tier;
    std::string compression_algorithm;  // for compression cases
    bool is_compression = false;        // flag
};

}  // namespace benchmark
