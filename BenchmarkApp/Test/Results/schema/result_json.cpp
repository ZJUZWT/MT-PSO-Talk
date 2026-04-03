#include "Results/schema/result_json.h"

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
           << "\"total_us\":" << result.total_us
           << '}';
    return stream.str();
}

std::string to_json(const CompressionResult& result) {
    std::ostringstream stream;
    stream << '{'
           << "\"platform\":\"" << escape_json(result.platform) << "\","
           << "\"algorithm\":\"" << escape_json(result.algorithm) << "\","
           << "\"baseline_package_id\":\"" << escape_json(result.baseline_package_id) << "\","
           << "\"decompress_us\":" << result.decompress_us << ','
           << "\"decompressed_output_hash\":\"" << escape_json(result.decompressed_output_hash) << "\""
           << '}';
    return stream.str();
}

}  // namespace benchmark
