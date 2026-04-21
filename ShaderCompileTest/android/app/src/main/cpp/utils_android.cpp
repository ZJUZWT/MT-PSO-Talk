#include "benchmark.h"
#include <android/asset_manager.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// ── Read binary asset (for SPIR-V) ─────────────────────────────────
std::vector<char> readAsset(AAssetManager* mgr, const std::string& filename) {
    AAsset* asset = AAssetManager_open(mgr, filename.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        throw std::runtime_error("Failed to open asset: " + filename);
    }
    size_t size = AAsset_getLength(asset);
    std::vector<char> buffer(size);
    AAsset_read(asset, buffer.data(), size);
    AAsset_close(asset);
    return buffer;
}

// ── Read text asset (for GLSL source) ───────────────────────────────
std::string readAssetText(AAssetManager* mgr, const std::string& filename) {
    auto data = readAsset(mgr, filename);
    return std::string(data.begin(), data.end());
}

// ── Format single result ────────────────────────────────────────────
std::string formatResult(const BenchmarkResult& r) {
    std::ostringstream ss;
    ss << "  Backend:    " << r.backend << "\n"
       << "  Test Case:  " << r.testCase << "\n"
       << "  GPU Time:   " << std::fixed << std::setprecision(4) << r.gpuTimeMs << " ms\n"
       << "  Draw Calls: " << r.drawCalls << "\n"
       << "  Status:     " << (r.success ? "OK" : "FAILED") << "\n";
    return ss.str();
}

// ── Format comparison ───────────────────────────────────────────────
std::string formatComparison(const BenchmarkResult& vk, const BenchmarkResult& gl) {
    std::ostringstream ss;
    ss << "================================================================\n"
       << "  PSO Dead-Code-Elimination Benchmark Results\n"
       << "================================================================\n\n";

    ss << "[Vulkan (PSO with colorWriteMask=0)]\n";
    ss << formatResult(vk);

    ss << "\n[OpenGL ES (glColorMask at draw time)]\n";
    ss << formatResult(gl);

    ss << "\n----------------------------------------------------------------\n";

    if (vk.success && gl.success && vk.gpuTimeMs > 0.0) {
        double speedup = gl.gpuTimeMs / vk.gpuTimeMs;
        ss << "  Speedup (GLES / VK): " << std::fixed << std::setprecision(2)
           << speedup << "x\n";

        if (speedup > 5.0) {
            ss << "\n  >> CONCLUSION: Vulkan PSO compiler successfully performed\n"
               << "     cross-stage Dead Code Elimination!\n"
               << "     The heavy VS computation was completely eliminated\n"
               << "     because colorWriteMask=0 was known at compile time.\n";
        } else if (speedup > 1.5) {
            ss << "\n  >> CONCLUSION: Vulkan shows moderate improvement.\n"
               << "     The driver may have performed partial optimization.\n";
        } else {
            ss << "\n  >> CONCLUSION: Minimal difference detected.\n"
               << "     Your GLES driver may be doing JIT recompilation,\n"
               << "     or the GPU has hardware-level color-mask culling.\n";
        }
    } else {
        ss << "  (Cannot compute speedup - one or both tests failed)\n";
    }

    ss << "================================================================\n";
    return ss.str();
}
