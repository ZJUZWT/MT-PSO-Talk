#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {

std::string export_graphics_json(const GraphicsResult& result);
std::string export_compression_json(const CompressionResult& result);

}  // namespace benchmark
