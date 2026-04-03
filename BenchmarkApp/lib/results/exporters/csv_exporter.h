#pragma once

#include <string>
#include <vector>

#include "results/schema/result_types.h"

namespace benchmark {

std::string graphics_csv_header();
std::string compression_csv_header();
std::string export_graphics_csv_row(const GraphicsResult& result);
std::string export_compression_csv_row(const CompressionResult& result);

}  // namespace benchmark
