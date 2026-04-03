#pragma once

#include <functional>
#include <string>
#include <vector>

#include "driver/common/timing_model.h"

namespace benchmark {

struct GraphicsBackendEntry {
    std::string mode;
    std::string platform;
    std::string api;
    std::string driver_mode;
    std::function<TimingProfile()> make_profile;
};

class GraphicsBackendRegistry {
public:
    static GraphicsBackendRegistry& instance() {
        static GraphicsBackendRegistry reg;
        return reg;
    }

    void add(GraphicsBackendEntry entry) {
        entries_.push_back(std::move(entry));
    }

    const std::vector<GraphicsBackendEntry>& all() const { return entries_; }

private:
    std::vector<GraphicsBackendEntry> entries_;
};

}  // namespace benchmark
