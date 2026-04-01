#include "Results/exporters/csv_exporter.h"

namespace benchmark {

std::string export_graphics_csv_row(const GraphicsResult& result) {
    return result.platform + "," + result.api + "," + result.driver_mode + "," +
           result.case_name + "," + std::to_string(result.total_us);
}

std::string export_compression_csv_row(const CompressionResult& result) {
    return result.platform + "," + result.algorithm + "," +
           result.baseline_package_id + "," +
           std::to_string(result.decompress_us) + "," +
           result.decompressed_output_hash;
}

}  // namespace benchmark
