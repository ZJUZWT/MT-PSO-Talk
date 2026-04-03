#include "results/exporters/csv_exporter.h"

namespace benchmark {

std::string graphics_csv_header() {
    return "platform,api,driver_mode,case_name,device_model,soc,os_version,"
           "mesa_version,iteration_index,cold_cache,status,total_us,"
           "shader_compile_us,link_pipeline_create_us,first_draw_ready_us";
}

std::string compression_csv_header() {
    return "platform,algorithm,baseline_package_id,device_model,soc,os_version,"
           "version,level,input_size,compressed_size,compression_ratio,"
           "compress_us,throughput_mbps,compressed_output_hash,"
           "decompress_us,decompressed_output_hash,status";
}

std::string export_graphics_csv_row(const GraphicsResult& result) {
    return result.platform + "," + result.api + "," + result.driver_mode + "," +
           result.case_name + "," + result.device_model + "," + result.soc + "," +
           result.os_version + "," + result.mesa_version + "," +
           std::to_string(result.iteration_index) + "," +
           (result.cold_cache ? "true" : "false") + "," +
           result.status + "," +
           std::to_string(result.total_us) + "," +
           std::to_string(result.shader_compile_us) + "," +
           std::to_string(result.link_pipeline_create_us) + "," +
           std::to_string(result.first_draw_ready_us);
}

std::string export_compression_csv_row(const CompressionResult& result) {
    return result.platform + "," + result.algorithm + "," +
           result.baseline_package_id + "," +
           result.device_model + "," + result.soc + "," +
           result.os_version + "," + result.version + "," +
           result.level + "," +
           std::to_string(result.input_size) + "," +
           std::to_string(result.compressed_size) + "," +
           std::to_string(result.compression_ratio) + "," +
           std::to_string(result.compress_us) + "," +
           std::to_string(result.throughput_mbps) + "," +
           result.compressed_output_hash + "," +
           std::to_string(result.decompress_us) + "," +
           result.decompressed_output_hash + "," +
           result.status;
}

}  // namespace benchmark
