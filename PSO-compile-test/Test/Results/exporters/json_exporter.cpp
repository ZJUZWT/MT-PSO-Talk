#include "Results/exporters/json_exporter.h"

#include "Results/schema/result_json.h"

namespace benchmark {

std::string export_graphics_json(const GraphicsResult& result) {
    return to_json(result);
}

std::string export_compression_json(const CompressionResult& result) {
    return to_json(result);
}

}  // namespace benchmark
