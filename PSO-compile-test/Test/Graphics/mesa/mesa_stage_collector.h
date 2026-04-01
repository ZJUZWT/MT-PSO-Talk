#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Results/schema/result_types.h"

namespace benchmark {

class MesaStageCollector {
public:
    void add_stage(const std::string& name, std::int64_t duration_us) {
        stages_.push_back(StageTiming{name, duration_us});
    }

    const std::vector<StageTiming>& stages() const {
        return stages_;
    }

private:
    std::vector<StageTiming> stages_;
};

}  // namespace benchmark
