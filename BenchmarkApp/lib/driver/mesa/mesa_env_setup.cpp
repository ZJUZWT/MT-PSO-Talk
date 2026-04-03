#include "driver/mesa/mesa_env_setup.h"

#include <cstdlib>
#include <fstream>
#include <regex>
#include <sstream>

namespace benchmark {

void setup_mesa_trace_env(const std::string& log_path) {
#if defined(__ANDROID__)
    // Enable Mesa shader compilation tracing
    setenv("MESA_SHADER_CAPTURE_PATH", log_path.c_str(), 1);
    setenv("MESA_DEBUG", "perf", 1);
    setenv("NIR_DEBUG", "print_internal", 1);
    setenv("MESA_LOG_FILE", log_path.c_str(), 1);
#else
    // No-op on non-Android platforms
    (void)log_path;
#endif
}

std::vector<StageTiming> parse_mesa_trace_log(const std::string& log_path) {
    std::vector<StageTiming> stages;

#if defined(__ANDROID__)
    std::ifstream file(log_path);
    if (!file.is_open()) return stages;

    // Parse lines like: "stage_name: 1234 us"
    std::regex timing_re(R"((\w+):\s+(\d+)\s*us)");
    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, timing_re)) {
            StageTiming st;
            st.name = match[1].str();
            st.duration_us = std::stoll(match[2].str());
            stages.push_back(st);
        }
    }
#else
    // No-op on non-Android platforms
    (void)log_path;
#endif

    return stages;
}

}  // namespace benchmark
