#pragma once

#include <cstdint>
#include <string>

namespace benchmark {

struct MesaTraceEvent {
    std::string name;
    std::int64_t duration_us = 0;
};

}  // namespace benchmark
