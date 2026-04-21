#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ── Result structure ────────────────────────────────────────────────
struct BenchmarkResult {
    std::string backend;          // "Vulkan" or "OpenGL ES"
    std::string testCase;
    double      gpuTimeMs;        // GPU-side elapsed time in milliseconds
    int         drawCalls;
    int         iterations;
    bool        success;
};

// Forward declaration for AAssetManager
struct AAssetManager;

// ── Vulkan benchmark ────────────────────────────────────────────────
BenchmarkResult runVulkanBenchmark(int iterations, AAssetManager* assetMgr, int loopCount = 5000);

// ── OpenGL ES benchmark ───────────────────────────────────────────
BenchmarkResult runGLESBenchmark(int iterations, int loopCount = 5000);

// ── Heavy shader + colorWriteMask ENABLED (normal write) ────────────
// Same heavy shader but with colorWriteMask fully enabled.
// Forces the GPU to actually execute the heavy computation.
BenchmarkResult runVulkanBenchmarkColorWrite(int iterations, AAssetManager* assetMgr, int loopCount = 5000);
BenchmarkResult runGLESBenchmarkColorWrite(int iterations, int loopCount = 5000);

// ── Baseline benchmarks (empty shaders, normal colorWriteMask) ──────
BenchmarkResult runVulkanBaseline(int iterations, AAssetManager* assetMgr);
BenchmarkResult runGLESBaseline(int iterations);
// ── Utilities ───────────────────────────────────────────────────────
std::vector<char> readAsset(AAssetManager* mgr, const std::string& filename);
std::string       readAssetText(AAssetManager* mgr, const std::string& filename);
std::string       formatResult(const BenchmarkResult& r);
std::string       formatComparison(const BenchmarkResult& vk, const BenchmarkResult& gl);
