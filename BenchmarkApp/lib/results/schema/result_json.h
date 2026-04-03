#pragma once

#include <string>

#include "results/schema/result_types.h"

namespace benchmark {

std::string to_json(const GraphicsResult& result);
std::string to_json(const CompressionResult& result);

}  // namespace benchmark
