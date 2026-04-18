#include "results/exporters/csv_exporter.h"

namespace benchmark {
namespace {

std::string escape_csv_field(const std::string& value) {
    if (value.find_first_of(",\"\n\r") == std::string::npos) {
        return value;
    }

    std::string escaped;
    escaped.reserve(value.size() + 2);
    escaped.push_back('"');
    for (char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped.push_back(ch);
        }
    }
    escaped.push_back('"');
    return escaped;
}

}  // namespace

std::string graphics_csv_header() {
    return "platform,api,driver_mode,case_name,device_model,soc,os_version,"
           "mesa_version,iteration_index,cold_cache,status,total_us,"
           "shader_compile_us,link_pipeline_create_us,first_draw_ready_us";
}

std::string compression_csv_header() {
    return "platform,algorithm,baseline_package_id,device_model,soc,os_version,"
           "version,level,payload_profile,iteration_index,input_size,compressed_size,"
           "compression_ratio,compress_us,throughput_mbps,input_hash,"
           "compressed_output_hash,decompress_us,decompressed_output_hash,"
           "roundtrip_hash_match,roundtrip_byte_match,status";
}

std::string export_graphics_csv_row(const GraphicsResult& result) {
    return escape_csv_field(result.platform) + "," +
           escape_csv_field(result.api) + "," +
           escape_csv_field(result.driver_mode) + "," +
           escape_csv_field(result.case_name) + "," +
           escape_csv_field(result.device_model) + "," +
           escape_csv_field(result.soc) + "," +
           escape_csv_field(result.os_version) + "," +
           escape_csv_field(result.mesa_version) + "," +
           std::to_string(result.iteration_index) + "," +
           (result.cold_cache ? "true" : "false") + "," +
           escape_csv_field(result.status) + "," +
           std::to_string(result.total_us) + "," +
           std::to_string(result.shader_compile_us) + "," +
           std::to_string(result.link_pipeline_create_us) + "," +
           std::to_string(result.first_draw_ready_us);
}

std::string export_compression_csv_row(const CompressionResult& result) {
    return escape_csv_field(result.platform) + "," +
           escape_csv_field(result.algorithm) + "," +
           escape_csv_field(result.baseline_package_id) + "," +
           escape_csv_field(result.device_model) + "," +
           escape_csv_field(result.soc) + "," +
           escape_csv_field(result.os_version) + "," +
           escape_csv_field(result.version) + "," +
           escape_csv_field(result.level) + "," +
           escape_csv_field(result.payload_profile) + "," +
           std::to_string(result.iteration_index) + "," +
           std::to_string(result.input_size) + "," +
           std::to_string(result.compressed_size) + "," +
           std::to_string(result.compression_ratio) + "," +
           std::to_string(result.compress_us) + "," +
           std::to_string(result.throughput_mbps) + "," +
           escape_csv_field(result.input_hash) + "," +
           escape_csv_field(result.compressed_output_hash) + "," +
           std::to_string(result.decompress_us) + "," +
           escape_csv_field(result.decompressed_output_hash) + "," +
           (result.roundtrip_hash_match ? "true" : "false") + std::string(",") +
           (result.roundtrip_byte_match ? "true" : "false") + std::string(",") +
           escape_csv_field(result.status);
}

}  // namespace benchmark
