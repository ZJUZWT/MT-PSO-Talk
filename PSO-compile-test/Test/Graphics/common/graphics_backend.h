#pragma once

#include <string>

#include "Results/schema/result_types.h"

namespace benchmark {

class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;
    virtual GraphicsResult run_compile_case(const std::string& workload_tier) = 0;
};

GraphicsBackend& make_stub_native_backend();

}  // namespace benchmark
