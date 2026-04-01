#pragma once

#include <chrono>
#include <cstdint>

namespace benchmark {

class TimingScope {
public:
    explicit TimingScope(std::int64_t& duration_us)
        : duration_us_(duration_us),
          start_(std::chrono::steady_clock::now()) {}

    ~TimingScope() {
        const auto end = std::chrono::steady_clock::now();
        duration_us_ = std::chrono::duration_cast<std::chrono::microseconds>(
                           end - start_)
                           .count();
    }

private:
    std::int64_t& duration_us_;
    std::chrono::steady_clock::time_point start_;
};

}  // namespace benchmark
