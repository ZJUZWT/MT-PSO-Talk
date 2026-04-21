#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ── Result structure ────────────────────────────────────────────────
struct BenchmarkResult {
    std::string backend;          // "Vulkan" or "OpenGL"
    std::string testCase;         // e.g. "ColorWriteMask=0"
    double      gpuTimeMs;        // GPU-side elapsed time in milliseconds
    int         drawCalls;        // number of draw calls issued
    int         iterations;       // number of measurement iterations
    bool        success;
};

// ── Vulkan benchmark ────────────────────────────────────────────────
// Runs the heavy-shader draw call with colorWriteMask = 0 under Vulkan PSO,
// measures pure GPU time via vkCmdWriteTimestamp.
BenchmarkResult runVulkanBenchmark(int iterations, int loopCount = 5000);

// ── OpenGL benchmark ────────────────────────────────────────────────
// Runs the same heavy-shader draw call with glColorMask(0,0,0,0) under OpenGL,
// measures pure GPU time via GL_TIME_ELAPSED query.
BenchmarkResult runOpenGLBenchmark(int iterations, int loopCount = 5000);

// ── Heavy shader + colorWriteMask ENABLED (normal write) ────────────
// Same heavy shader but with colorWriteMask fully enabled.
// This forces the GPU to actually execute the heavy computation.
// Used to compare against colorWriteMask=0 to prove DCE optimization.
BenchmarkResult runVulkanBenchmarkColorWrite(int iterations, int loopCount = 5000);
BenchmarkResult runOpenGLBenchmarkColorWrite(int iterations, int loopCount = 5000);

// ── Baseline benchmarks (empty shaders, normal colorWriteMask) ──────
// These measure the bare minimum pipeline overhead with trivial shaders
// that do nothing but output a constant color. Used as a reference.
BenchmarkResult runVulkanBaseline(int iterations);
BenchmarkResult runOpenGLBaseline(int iterations);

// ── Utilities ───────────────────────────────────────────────────────
std::vector<char> readFile(const std::string& filepath);
std::string        readTextFile(const std::string& filepath);
void               printResult(const BenchmarkResult& r);
void               printComparison(const BenchmarkResult& vk, const BenchmarkResult& gl);
