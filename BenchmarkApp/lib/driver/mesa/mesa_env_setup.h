#pragma once

#include <string>
#include <vector>

#include "results/schema/result_types.h"

namespace benchmark {

// Set Mesa environment variables for tracing shader compilation
void setup_mesa_trace_env(const std::string& log_path);

// Parse Mesa trace log after benchmark run and extract stage timings
std::vector<StageTiming> parse_mesa_trace_log(const std::string& log_path);

}  // namespace benchmark
