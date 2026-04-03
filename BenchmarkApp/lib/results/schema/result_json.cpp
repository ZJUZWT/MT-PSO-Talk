#include "results/schema/result_json.h"

#include <sstream>

namespace benchmark {
namespace {

std::string escape_json(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());

    for (char ch : value) {
        if (ch == '\\' || ch == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }

    return escaped;
}

}  // namespace

std::string to_json(const GraphicsResult& result) {
    std::ostringstream stream;
    stream << '{'
           << "\"platform\":\"" << escape_json(result.platform) << "\","
           << "\"api\":\"" << escape_json(result.api) << "\","
           << "\"driver_mode\":\"" << escape_json(result.driver_mode) << "\","
           << "\"case_name\":\"" << escape_json(result.case_name) << "\","
           << "\"device_model\":\"" << escape_json(result.device_model) << "\","
           << "\"soc\":\"" << escape_json(result.soc) << "\","
           << "\"os_version\":\"" << escape_json(result.os_version) << "\","
           << "\"mesa_version\":\"" << escape_json(result.mesa_version) << "\","
           << "\"iteration_index\":" << result.iteration_index << ','
           << "\"cold_cache\":" << (result.cold_cache ? "true" : "false") << ','
           << "\"status\":\"" << escape_json(result.status) << "\","
           << "\"total_us\":" << result.total_us << ','
           << "\"shader_compile_us\":" << result.shader_compile_us << ','
           << "\"link_pipeline_create_us\":" << result.link_pipeline_create_us << ','
           << "\"first_draw_ready_us\":" << result.first_draw_ready_us << ','
           << "\"stage_breakdown\":[";

    for (std::size_t i = 0; i < result.stage_breakdown.size(); ++i) {
        if (i > 0) stream << ',';
        stream << "{\"name\":\"" << escape_json(result.stage_breakdown[i].name) << "\","
               << "\"duration_us\":" << result.stage_breakdown[i].duration_us << '}';
    }

    stream << ']' << '}';
    return stream.str();
}

std::string to_json(const CompressionResult& result) {
    std::ostringstream stream;
    stream << '{'
           << "\"platform\":\"" << escape_json(result.platform) << "\","
           << "\"algorithm\":\"" << escape_json(result.algorithm) << "\","
           << "\"baseline_package_id\":\"" << escape_json(result.baseline_package_id) << "\","
           << "\"device_model\":\"" << escape_json(result.device_model) << "\","
           << "\"soc\":\"" << escape_json(result.soc) << "\","
           << "\"os_version\":\"" << escape_json(result.os_version) << "\","
           << "\"version\":\"" << escape_json(result.version) << "\","
           << "\"level\":\"" << escape_json(result.level) << "\","
           << "\"input_size\":" << result.input_size << ','
           << "\"compressed_size\":" << result.compressed_size << ','
           << "\"compression_ratio\":" << result.compression_ratio << ','
           << "\"compress_us\":" << result.compress_us << ','
           << "\"throughput_mbps\":" << result.throughput_mbps << ','
           << "\"compressed_output_hash\":\"" << escape_json(result.compressed_output_hash) << "\","
           << "\"decompress_us\":" << result.decompress_us << ','
           << "\"decompressed_output_hash\":\"" << escape_json(result.decompressed_output_hash) << "\","
           << "\"status\":\"" << escape_json(result.status) << "\""
           << '}';
    return stream.str();
}

}  // namespace benchmark
