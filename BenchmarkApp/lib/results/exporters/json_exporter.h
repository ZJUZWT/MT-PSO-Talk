#pragma once

#include <string>
#include <vector>

#include "results/schema/result_types.h"

namespace benchmark {

std::string export_graphics_json(const GraphicsResult& result);
std::string export_compression_json(const CompressionResult& result);
std::string export_graphics_json_array(const std::vector<GraphicsResult>& results);
std::string export_compression_json_array(const std::vector<CompressionResult>& results);

}  // namespace benchmark
