#include "benchmark.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

// ── Read binary file (for SPIR-V) ──────────────────────────────────
std::vector<char> readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

// ── Read text file (for GLSL source) ───────────────────────────────
std::string readTextFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ── Print single result ────────────────────────────────────────────
void printResult(const BenchmarkResult& r) {
    std::cout << "  Backend:    " << r.backend << "\n"
              << "  Test Case:  " << r.testCase << "\n"
              << "  GPU Time:   " << std::fixed << std::setprecision(4) << r.gpuTimeMs << " ms\n"
              << "  Draw Calls: " << r.drawCalls << "\n"
              << "  Iterations: " << r.iterations << "\n"
              << "  Status:     " << (r.success ? "OK" : "FAILED") << "\n";
}

// ── Print comparison ───────────────────────────────────────────────
void printComparison(const BenchmarkResult& vk, const BenchmarkResult& gl) {
    std::cout << "\n"
              << "================================================================\n"
              << "  PSO Dead-Code-Elimination Benchmark Results\n"
              << "================================================================\n\n";

    std::cout << "[Vulkan (PSO with colorWriteMask=0)]\n";
    printResult(vk);

    std::cout << "\n[OpenGL (glColorMask(0,0,0,0) at draw time)]\n";
    printResult(gl);

    std::cout << "\n----------------------------------------------------------------\n";

    if (vk.success && gl.success && gl.gpuTimeMs > 0.0) {
        double speedup = gl.gpuTimeMs / vk.gpuTimeMs;
        std::cout << "  Speedup (GL / VK): " << std::fixed << std::setprecision(2)
                  << speedup << "x\n";

        if (speedup > 5.0) {
            std::cout << "\n  >> CONCLUSION: Vulkan PSO compiler successfully performed\n"
                      << "     cross-stage Dead Code Elimination!\n"
                      << "     The 5000-iteration sin/cos loop in the VS was completely\n"
                      << "     eliminated because colorWriteMask=0 was known at compile time.\n";
        } else if (speedup > 1.5) {
            std::cout << "\n  >> CONCLUSION: Vulkan shows moderate improvement.\n"
                      << "     The driver may have performed partial optimization.\n";
        } else {
            std::cout << "\n  >> CONCLUSION: Minimal difference detected.\n"
                      << "     Your GL driver may be doing JIT recompilation,\n"
                      << "     or the GPU has hardware-level color-mask culling.\n";
        }
    } else {
        std::cout << "  (Cannot compute speedup – one or both tests failed)\n";
    }

    std::cout << "================================================================\n\n";
}
