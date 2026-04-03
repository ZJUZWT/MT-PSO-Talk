#include "Mesa/adapters/mesa_trace_parser.h"

#include <string_view>

namespace benchmark {

namespace {

std::string extract_value(std::string_view line, std::string_view key) {
    const std::string token = std::string(key) + "=";
    const std::size_t start = line.find(token);
    if (start == std::string_view::npos) {
        return {};
    }

    const std::size_t value_start = start + token.size();
    const std::size_t value_end = line.find(' ', value_start);
    return std::string(line.substr(value_start, value_end - value_start));
}

}  // namespace

std::vector<MesaTraceEvent> parse_mesa_trace_lines(
    const std::vector<std::string>& lines) {
    std::vector<MesaTraceEvent> events;
    events.reserve(lines.size());

    for (const std::string& line : lines) {
        const std::string name = extract_value(line, "stage");
        const std::string duration = extract_value(line, "duration_us");

        if (name.empty() || duration.empty()) {
            continue;
        }

        events.push_back(MesaTraceEvent{name, std::stoll(duration)});
    }

    return events;
}

}  // namespace benchmark
