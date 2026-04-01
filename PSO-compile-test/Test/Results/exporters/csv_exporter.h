#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {

std::string export_graphics_csv_row(const GraphicsResult& result);
std::string export_compression_csv_row(const CompressionResult& result);

}  // namespace benchmark
