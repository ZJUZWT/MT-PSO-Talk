#pragma once

#include <string>
#include <vector>

#include "driver/mesa/mesa_trace_event.h"

namespace benchmark {

std::vector<MesaTraceEvent> parse_mesa_trace_lines(
    const std::vector<std::string>& lines);

}  // namespace benchmark
