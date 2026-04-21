// ====================================================================
// benchmark_main.cpp – JNI bridge for Android benchmark
//
// Provides native methods called from BenchmarkActivity.java.
// Runs both Vulkan and GLES benchmarks at multiple loop counts
// (10, 100, 500, 5000) and returns a formatted comparison table.
// ====================================================================

#include "benchmark.h"

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vulkan/vulkan.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#define LOGTAG "PSO-Bench"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOGTAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__)

extern "C" {

// ── Run the full benchmark and return a formatted result string ─────
JNIEXPORT jstring JNICALL
Java_com_psobench_shadercompiletest_BenchmarkActivity_runBenchmark(
    JNIEnv* env, jobject /* this */, jobject assetManagerObj, jint iterations)
{
    AAssetManager* assetMgr = AAssetManager_fromJava(env, assetManagerObj);
    if (!assetMgr) {
        return env->NewStringUTF("ERROR: Failed to get AssetManager");
    }

    // Fixed draw call count – the variable under test is shader loopCount,
    // not the number of draw calls. Ignore the iterations parameter.
    const int iters = 100;

    // Loop counts to test
    std::vector<int> loopCounts = { 10, 100, 500, 5000 };

    LOGI("Starting multi-loopCount benchmark with %d draw calls...", iters);

    std::ostringstream output;
    output << "================================================================\n"
           << "  PSO Shader Compile Optimization Benchmark (Android)\n"
           << "  Test: Cross-stage DCE via colorWriteMask = 0\n"
           << "  Draw calls per test: " << iters << " (fixed)\n"
           << "  Vertices per draw: 6144 (32x32 grid)\n"
           << "  Shader loopCount: 10, 100, 500, 5000 (variable)\n"
           << "================================================================\n\n";

    // ── Phase 0: Baseline tests ────────────────────────────────────
    output << "=== PHASE 0: Baseline (empty shader, normal colorWriteMask) ===\n\n";

    LOGI("  [Vulkan Baseline] Running...");
    output << "  [Vulkan Baseline] Running...\n";
    BenchmarkResult vkBase = runVulkanBaseline(iters, assetMgr);
    LOGI("  [Vulkan Baseline] GPU Time: %.4f ms  %s", vkBase.gpuTimeMs, vkBase.success ? "OK" : "FAILED");
    output << "  [Vulkan Baseline] GPU Time: " << std::fixed << std::setprecision(4)
           << vkBase.gpuTimeMs << " ms  " << (vkBase.success ? "OK" : "FAILED") << "\n";

    LOGI("  [GLES Baseline] Running...");
    output << "  [GLES Baseline]   Running...\n";
    BenchmarkResult glBase = runGLESBaseline(iters);
    LOGI("  [GLES Baseline] GPU Time: %.4f ms  %s", glBase.gpuTimeMs, glBase.success ? "OK" : "FAILED");
    output << "  [GLES Baseline]   GPU Time: " << std::fixed << std::setprecision(4)
           << glBase.gpuTimeMs << " ms  " << (glBase.success ? "OK" : "FAILED") << "\n\n";

    // ── Phase 1: Heavy shader + colorWriteMask=0 ───────────────────
    output << "=== PHASE 1: Heavy shader + colorWriteMask=0 ===\n\n";

    // Storage for results
    struct TestRow {
        int    loopCount;
        double vkTimeMs;
        double glTimeMs;
        double speedup;
        bool   vkOk;
        bool   glOk;
    };
    std::vector<TestRow> rows;

    for (int lc : loopCounts) {
        LOGI("--- loopCount = %d ---", lc);
        output << "--- loopCount = " << lc << " ---\n";

        LOGI("  [Vulkan] Running...");
        output << "  [Vulkan] Running...\n";
        BenchmarkResult vk = runVulkanBenchmark(iters, assetMgr, lc);
        LOGI("  [Vulkan] GPU Time: %.4f ms  %s", vk.gpuTimeMs, vk.success ? "OK" : "FAILED");
        output << "  [Vulkan] GPU Time: " << std::fixed << std::setprecision(4)
               << vk.gpuTimeMs << " ms  " << (vk.success ? "OK" : "FAILED") << "\n";

        LOGI("  [GLES] Running...");
        output << "  [GLES]   Running...\n";
        BenchmarkResult gl = runGLESBenchmark(iters, lc);
        LOGI("  [GLES] GPU Time: %.4f ms  %s", gl.gpuTimeMs, gl.success ? "OK" : "FAILED");
        output << "  [GLES]   GPU Time: " << std::fixed << std::setprecision(4)
               << gl.gpuTimeMs << " ms  " << (gl.success ? "OK" : "FAILED") << "\n\n";

        TestRow row;
        row.loopCount = lc;
        row.vkTimeMs  = vk.gpuTimeMs;
        row.glTimeMs  = gl.gpuTimeMs;
        row.vkOk      = vk.success;
        row.glOk      = gl.success;
        row.speedup   = (vk.success && gl.success && vk.gpuTimeMs > 0.0)
                        ? gl.gpuTimeMs / vk.gpuTimeMs : 0.0;
        rows.push_back(row);
    }

    // ── Phase 2: Heavy shader + colorWriteMask=RGBA (full write) ───
    output << "=== PHASE 2: Heavy shader + colorWriteMask=RGBA (full write) ===\n\n";

    std::vector<TestRow> rowsCW; // CW = ColorWrite enabled

    for (int lc : loopCounts) {
        LOGI("--- CW loopCount = %d ---", lc);
        output << "--- loopCount = " << lc << " ---\n";

        LOGI("  [Vulkan CW] Running...");
        output << "  [Vulkan CW] Running...\n";
        BenchmarkResult vk = runVulkanBenchmarkColorWrite(iters, assetMgr, lc);
        LOGI("  [Vulkan CW] GPU Time: %.4f ms  %s", vk.gpuTimeMs, vk.success ? "OK" : "FAILED");
        output << "  [Vulkan CW] GPU Time: " << std::fixed << std::setprecision(4)
               << vk.gpuTimeMs << " ms  " << (vk.success ? "OK" : "FAILED") << "\n";

        LOGI("  [GLES CW] Running...");
        output << "  [GLES CW]   Running...\n";
        BenchmarkResult gl = runGLESBenchmarkColorWrite(iters, lc);
        LOGI("  [GLES CW] GPU Time: %.4f ms  %s", gl.gpuTimeMs, gl.success ? "OK" : "FAILED");
        output << "  [GLES CW]   GPU Time: " << std::fixed << std::setprecision(4)
               << gl.gpuTimeMs << " ms  " << (gl.success ? "OK" : "FAILED") << "\n\n";

        TestRow row;
        row.loopCount = lc;
        row.vkTimeMs  = vk.gpuTimeMs;
        row.glTimeMs  = gl.gpuTimeMs;
        row.vkOk      = vk.success;
        row.glOk      = gl.success;
        row.speedup   = (vk.success && gl.success && vk.gpuTimeMs > 0.0)
                        ? gl.gpuTimeMs / vk.gpuTimeMs : 0.0;
        rowsCW.push_back(row);
    }

    // ── Summary table ──────────────────────────────────────────────
    output << "\n"
           << "================================================================\n"
           << "  Summary: Vulkan PSO vs GLES - GPU Time (ms)\n"
           << "  Draw calls: " << iters
           << " | Vertices/draw: 6144\n"
           << "================================================================\n\n";

    // Baseline row
    output << "  --- Baseline (empty shader, normal colorWriteMask) ---\n";
    output << "  " << std::left << std::setw(12) << "Baseline"
           << std::right << std::fixed << std::setprecision(4)
           << std::setw(14) << vkBase.gpuTimeMs
           << std::setw(14) << glBase.gpuTimeMs
           << std::setw(12) << " (ref)"
           << "\n\n";

    // Phase 1: Heavy shader + colorWriteMask=0
    output << "  --- Phase 1: Heavy shader + colorWriteMask=0 (DCE test) ---\n";
    output << "  " << std::left << std::setw(12) << "LoopCount"
           << std::right << std::setw(14) << "Vulkan (ms)"
           << std::setw(14) << "GLES (ms)"
           << std::setw(12) << "GL/VK"
           << "\n";
    output << "  " << std::string(52, '-') << "\n";

    for (const auto& r : rows) {
        output << "  " << std::left << std::setw(12) << r.loopCount
               << std::right << std::fixed << std::setprecision(4)
               << std::setw(14) << r.vkTimeMs
               << std::setw(14) << r.glTimeMs;
        if (r.speedup > 0.0) {
            output << std::setw(10) << std::setprecision(2) << r.speedup << "x";
        } else {
            output << std::setw(12) << "N/A";
        }
        output << "\n";
    }
    output << "  " << std::string(52, '-') << "\n\n";

    // Phase 2: Heavy shader + colorWriteMask=RGBA
    output << "  --- Phase 2: Heavy shader + colorWriteMask=RGBA (full write) ---\n";
    output << "  " << std::left << std::setw(12) << "LoopCount"
           << std::right << std::setw(14) << "Vulkan (ms)"
           << std::setw(14) << "GLES (ms)"
           << std::setw(12) << "GL/VK"
           << "\n";
    output << "  " << std::string(52, '-') << "\n";

    for (const auto& r : rowsCW) {
        output << "  " << std::left << std::setw(12) << r.loopCount
               << std::right << std::fixed << std::setprecision(4)
               << std::setw(14) << r.vkTimeMs
               << std::setw(14) << r.glTimeMs;
        if (r.speedup > 0.0) {
            output << std::setw(10) << std::setprecision(2) << r.speedup << "x";
        } else {
            output << std::setw(12) << "N/A";
        }
        output << "\n";
    }
    output << "  " << std::string(52, '-') << "\n\n";

    // Cross-phase comparison
    output << "  --- Cross-Phase Comparison (mask=0 vs RGBA) ---\n";
    output << "  " << std::left << std::setw(12) << "LoopCount"
           << std::right << std::setw(14) << "VK mask=0"
           << std::setw(14) << "VK mask=RGBA"
           << std::setw(14) << "GL mask=0"
           << std::setw(14) << "GL mask=RGBA"
           << "\n";
    output << "  " << std::string(68, '-') << "\n";

    for (size_t i = 0; i < rows.size() && i < rowsCW.size(); i++) {
        output << "  " << std::left << std::setw(12) << rows[i].loopCount
               << std::right << std::fixed << std::setprecision(4)
               << std::setw(14) << rows[i].vkTimeMs
               << std::setw(14) << rowsCW[i].vkTimeMs
               << std::setw(14) << rows[i].glTimeMs
               << std::setw(14) << rowsCW[i].glTimeMs
               << "\n";
    }
    output << "  " << std::string(68, '-') << "\n\n";

    // ── Analysis ───────────────────────────────────────────────────
    if (rows.size() >= 2 && rows.front().glOk && rows.back().glOk) {
        double glGrowth = rows.back().glTimeMs / (rows.front().glTimeMs > 0 ? rows.front().glTimeMs : 1.0);
        double vkGrowth = rows.back().vkTimeMs / (rows.front().vkTimeMs > 0 ? rows.front().vkTimeMs : 1.0);

        output << "  Analysis (Phase 1 - colorWriteMask=0):\n"
               << "    Vulkan Baseline (empty shader):      "
               << std::fixed << std::setprecision(4) << vkBase.gpuTimeMs << " ms\n"
               << "    GLES Baseline (empty shader):        "
               << std::fixed << std::setprecision(4) << glBase.gpuTimeMs << " ms\n"
               << "    GLES GPU time growth (10 -> 5000):   "
               << std::fixed << std::setprecision(1) << glGrowth << "x\n"
               << "    Vulkan GPU time growth (10 -> 5000): "
               << std::fixed << std::setprecision(1) << vkGrowth << "x\n\n";

        if (vkBase.success && rows.back().vkOk) {
            double vkHeavyVsBase = rows.back().vkTimeMs / (vkBase.gpuTimeMs > 0 ? vkBase.gpuTimeMs : 1.0);
            output << "    Vulkan heavy(5000) mask=0 / baseline: "
                   << std::fixed << std::setprecision(2) << vkHeavyVsBase << "x\n";
            if (vkHeavyVsBase < 2.0) {
                output << "    -> Vulkan PSO DCE eliminated the heavy loop entirely!\n";
            }
        }

        if (glBase.success && rows.back().glOk) {
            double glHeavyVsBase = rows.back().glTimeMs / (glBase.gpuTimeMs > 0 ? glBase.gpuTimeMs : 1.0);
            output << "    GLES heavy(5000) mask=0 / baseline:   "
                   << std::fixed << std::setprecision(2) << glHeavyVsBase << "x\n";
            if (glHeavyVsBase > 5.0) {
                output << "    -> GLES still executes the heavy loop (no cross-stage DCE)\n";
            }
        }
        output << "\n";
    }

    if (rowsCW.size() >= 2 && rowsCW.front().vkOk && rowsCW.back().vkOk) {
        double vkCWGrowth = rowsCW.back().vkTimeMs / (rowsCW.front().vkTimeMs > 0 ? rowsCW.front().vkTimeMs : 1.0);
        double glCWGrowth = rowsCW.back().glTimeMs / (rowsCW.front().glTimeMs > 0 ? rowsCW.front().glTimeMs : 1.0);

        output << "  Analysis (Phase 2 - colorWriteMask=RGBA):\n"
               << "    Vulkan GPU time growth (10 -> 5000): "
               << std::fixed << std::setprecision(1) << vkCWGrowth << "x\n"
               << "    GLES GPU time growth (10 -> 5000):   "
               << std::fixed << std::setprecision(1) << glCWGrowth << "x\n";

        if (!rows.empty() && rows.back().vkOk && rowsCW.back().vkOk && rows.back().vkTimeMs > 0) {
            double vkDCERatio = rowsCW.back().vkTimeMs / rows.back().vkTimeMs;
            output << "    Vulkan mask=RGBA / mask=0 at loop=5000: "
                   << std::fixed << std::setprecision(1) << vkDCERatio << "x\n";
            if (vkDCERatio > 5.0) {
                output << "    -> HUGE gap! Vulkan PSO DCE saved "
                       << std::fixed << std::setprecision(1)
                       << (1.0 - 1.0/vkDCERatio) * 100.0 << "% GPU time!\n";
            }
        }

        if (!rows.empty() && rows.back().glOk && rowsCW.back().glOk && rows.back().glTimeMs > 0) {
            double glDCERatio = rowsCW.back().glTimeMs / rows.back().glTimeMs;
            output << "    GLES mask=RGBA / mask=0 at loop=5000:   "
                   << std::fixed << std::setprecision(1) << glDCERatio << "x\n";
            if (glDCERatio < 1.5) {
                output << "    -> GLES shows similar time with/without mask\n"
                       << "       (confirms: GLES cannot DCE based on runtime state)\n";
            }
        }
        output << "\n";
    }

    // ── Final conclusion ───────────────────────────────────────────
    output << "  >> CONCLUSION:\n";
    if (rows.size() >= 2 && rowsCW.size() >= 2) {
        bool vkDCEWorks = false;
        bool glNoDCE    = false;

        if (rows.back().vkOk && rowsCW.back().vkOk && rows.back().vkTimeMs > 0) {
            double vkDCERatio = rowsCW.back().vkTimeMs / rows.back().vkTimeMs;
            if (vkDCERatio > 3.0) vkDCEWorks = true;
        }
        if (rows.back().glOk && rowsCW.back().glOk && rows.back().glTimeMs > 0) {
            double glDCERatio = rowsCW.back().glTimeMs / rows.back().glTimeMs;
            if (glDCERatio < 2.0) glNoDCE = true;
        }

        if (vkDCEWorks && glNoDCE) {
            output << "     Vulkan PSO cross-stage DCE is PROVEN:\n"
                   << "       - Vulkan mask=0 is dramatically faster than mask=RGBA\n"
                   << "         (compiler eliminated dead code when output is unused)\n"
                   << "       - GLES mask=0 is similar to mask=RGBA\n"
                   << "         (runtime state cannot trigger compile-time DCE)\n";
        } else if (vkDCEWorks) {
            output << "     Vulkan PSO DCE optimization is effective.\n"
                   << "     GLES results are inconclusive.\n";
        } else {
            output << "     Results are inconclusive. The mobile GPU driver may be\n"
                   << "     doing hardware-level optimization in both cases.\n";
        }
    }

    output << "\n================================================================\n";

    std::string resultStr = output.str();
    LOGI("\n%s", resultStr.c_str());

    return env->NewStringUTF(resultStr.c_str());
}

// ── Quick check: does this device support Vulkan? ──────────────────
JNIEXPORT jboolean JNICALL
Java_com_psobench_shadercompiletest_BenchmarkActivity_isVulkanSupported(
    JNIEnv* /* env */, jobject /* this */)
{
    // Try to create a Vulkan instance
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ci.pApplicationInfo = &appInfo;

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&ci, nullptr, &instance);

    if (result == VK_SUCCESS && instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        return JNI_TRUE;
    }
    return JNI_FALSE;
}

} // extern "C"
