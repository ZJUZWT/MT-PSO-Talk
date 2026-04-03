#pragma once

#include <cstdint>
#include <string>

namespace benchmark {

struct GLCompileResult {
    int64_t compile_vertex_us = 0;
    int64_t compile_fragment_us = 0;
    int64_t link_program_us = 0;
    int64_t total_us = 0;
    std::string status = "passed";
};

#if defined(__ANDROID__)
GLCompileResult compile_gl_program(
    const std::string& vert_source,
    const std::string& frag_source);
#endif

}  // namespace benchmark
