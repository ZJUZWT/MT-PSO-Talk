// ====================================================================
// main.cpp – PSO Dead-Code-Elimination Benchmark
//
// Runs Vulkan PSO vs OpenGL comparison at multiple shader loop counts
// (10, 100, 500, 5000) to show how GPU time scales differently.
//
// Also runs a BASELINE test (empty shader, normal colorWriteMask)
// to establish the bare minimum pipeline overhead for comparison.
//
// Expected: Vulkan GPU time stays near-zero (DCE removes the loop),
//           OpenGL GPU time grows linearly with loop count.
// ====================================================================

#include "benchmark.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>

int main(int argc, char* argv[]) {
    // Fixed draw call count – the variable under test is shader loopCount,
    // not the number of draw calls.
    const int drawCalls = 100;

    // Loop counts to test
    std::vector<int> loopCounts = { 10, 100, 500, 5000 };

    std::cout << "\n"
              << "================================================================\n"
              << "  PSO Shader Compile Optimization Benchmark\n"
              << "  Test: Cross-stage DCE via colorWriteMask = 0\n"
              << "  Draw calls per test: " << drawCalls << " (fixed)\n"
              << "  Vertices per draw: 6144 (32x32 grid)\n"
              << "  Shader loopCount: 10, 100, 500, 5000 (variable)\n"
              << "================================================================\n\n";

    // ── Phase 0: Baseline tests (empty shader, normal colorWriteMask) ──
    std::cout << "=== PHASE 0: Baseline (empty shader, normal colorWriteMask) ===\n\n";

    std::cout << "  [Vulkan Baseline] Running...\n";
    BenchmarkResult vkBase = runVulkanBaseline(drawCalls);
    std::cout << "  [Vulkan Baseline] GPU Time: " << std::fixed << std::setprecision(4)
              << vkBase.gpuTimeMs << " ms  " << (vkBase.success ? "OK" : "FAILED") << "\n";

    std::cout << "  [OpenGL Baseline] Running...\n";
    BenchmarkResult glBase = runOpenGLBaseline(drawCalls);
    std::cout << "  [OpenGL Baseline] GPU Time: " << std::fixed << std::setprecision(4)
              << glBase.gpuTimeMs << " ms  " << (glBase.success ? "OK" : "FAILED") << "\n\n";

    // ── Phase 1: Heavy shader + colorWriteMask=0 at multiple loop counts ──
    std::cout << "=== PHASE 1: Heavy shader + colorWriteMask=0 ===\n\n";

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
        std::cout << "--- loopCount = " << lc << " ---\n";

        std::cout << "  [Vulkan] Running...\n";
        BenchmarkResult vk = runVulkanBenchmark(drawCalls, lc);
        std::cout << "  [Vulkan] GPU Time: " << std::fixed << std::setprecision(4)
                  << vk.gpuTimeMs << " ms  " << (vk.success ? "OK" : "FAILED") << "\n";

        std::cout << "  [OpenGL] Running...\n";
        BenchmarkResult gl = runOpenGLBenchmark(drawCalls, lc);
        std::cout << "  [OpenGL] GPU Time: " << std::fixed << std::setprecision(4)
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

    // ── Phase 2: Heavy shader + colorWriteMask=RGBA (full write) ──────
    std::cout << "=== PHASE 2: Heavy shader + colorWriteMask=RGBA (full write) ===\n\n";

    std::vector<TestRow> rowsCW; // CW = ColorWrite enabled

    for (int lc : loopCounts) {
        std::cout << "--- loopCount = " << lc << " ---\n";

        std::cout << "  [Vulkan CW] Running...\n";
        BenchmarkResult vk = runVulkanBenchmarkColorWrite(drawCalls, lc);
        std::cout << "  [Vulkan CW] GPU Time: " << std::fixed << std::setprecision(4)
                  << vk.gpuTimeMs << " ms  " << (vk.success ? "OK" : "FAILED") << "\n";

        std::cout << "  [OpenGL CW] Running...\n";
        BenchmarkResult gl = runOpenGLBenchmarkColorWrite(drawCalls, lc);
        std::cout << "  [OpenGL CW] GPU Time: " << std::fixed << std::setprecision(4)
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

    // ── Print summary table ────────────────────────────────────────
    std::cout << "\n"
              << "================================================================\n"
              << "  Summary: Vulkan PSO vs OpenGL – GPU Time (ms)\n"
              << "  Draw calls: " << drawCalls
              << " | Vertices/draw: 6144\n"
              << "================================================================\n\n";

    // Baseline row
    std::cout << "  --- Baseline (empty shader, normal colorWriteMask) ---\n";
    std::cout << "  " << std::left << std::setw(12) << "Baseline"
              << std::right << std::fixed << std::setprecision(4)
              << std::setw(14) << vkBase.gpuTimeMs
              << std::setw(14) << glBase.gpuTimeMs
              << std::setw(12) << " (ref)"
              << "\n\n";

    // Phase 1: Heavy shader + colorWriteMask=0
    std::cout << "  --- Phase 1: Heavy shader + colorWriteMask=0 (DCE test) ---\n";
    std::cout << "  " << std::left << std::setw(12) << "LoopCount"
              << std::right << std::setw(14) << "Vulkan (ms)"
              << std::setw(14) << "OpenGL (ms)"
              << std::setw(12) << "GL/VK"
              << "\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    for (const auto& r : rows) {
        std::cout << "  " << std::left << std::setw(12) << r.loopCount
                  << std::right << std::fixed << std::setprecision(4)
                  << std::setw(14) << r.vkTimeMs
                  << std::setw(14) << r.glTimeMs;
        if (r.speedup > 0.0) {
            std::cout << std::setw(10) << std::setprecision(2) << r.speedup << "x";
        } else {
            std::cout << std::setw(12) << "N/A";
        }
        std::cout << "\n";
    }
    std::cout << "  " << std::string(52, '-') << "\n\n";

    // Phase 2: Heavy shader + colorWriteMask=RGBA
    std::cout << "  --- Phase 2: Heavy shader + colorWriteMask=RGBA (full write) ---\n";
    std::cout << "  " << std::left << std::setw(12) << "LoopCount"
              << std::right << std::setw(14) << "Vulkan (ms)"
              << std::setw(14) << "OpenGL (ms)"
              << std::setw(12) << "GL/VK"
              << "\n";
    std::cout << "  " << std::string(52, '-') << "\n";

    for (const auto& r : rowsCW) {
        std::cout << "  " << std::left << std::setw(12) << r.loopCount
                  << std::right << std::fixed << std::setprecision(4)
                  << std::setw(14) << r.vkTimeMs
                  << std::setw(14) << r.glTimeMs;
        if (r.speedup > 0.0) {
            std::cout << std::setw(10) << std::setprecision(2) << r.speedup << "x";
        } else {
            std::cout << std::setw(12) << "N/A";
        }
        std::cout << "\n";
    }
    std::cout << "  " << std::string(52, '-') << "\n\n";

    // ── Cross-phase comparison (the key insight) ───────────────────
    std::cout << "  --- Cross-Phase Comparison (colorWriteMask=0 vs RGBA) ---\n";
    std::cout << "  " << std::left << std::setw(12) << "LoopCount"
              << std::right << std::setw(14) << "VK mask=0"
              << std::setw(14) << "VK mask=RGBA"
              << std::setw(14) << "GL mask=0"
              << std::setw(14) << "GL mask=RGBA"
              << "\n";
    std::cout << "  " << std::string(68, '-') << "\n";

    for (size_t i = 0; i < rows.size() && i < rowsCW.size(); i++) {
        std::cout << "  " << std::left << std::setw(12) << rows[i].loopCount
                  << std::right << std::fixed << std::setprecision(4)
                  << std::setw(14) << rows[i].vkTimeMs
                  << std::setw(14) << rowsCW[i].vkTimeMs
                  << std::setw(14) << rows[i].glTimeMs
                  << std::setw(14) << rowsCW[i].glTimeMs
                  << "\n";
    }
    std::cout << "  " << std::string(68, '-') << "\n\n";

    // ── Analysis ───────────────────────────────────────────────────
    if (rows.size() >= 2 && rows.front().glOk && rows.back().glOk) {
        double glGrowth = rows.back().glTimeMs / (rows.front().glTimeMs > 0 ? rows.front().glTimeMs : 1.0);
        double vkGrowth = rows.back().vkTimeMs / (rows.front().vkTimeMs > 0 ? rows.front().vkTimeMs : 1.0);

        std::cout << "  Analysis (Phase 1 – colorWriteMask=0):\n"
                  << "    Vulkan Baseline (empty shader):      "
                  << std::fixed << std::setprecision(4) << vkBase.gpuTimeMs << " ms\n"
                  << "    OpenGL Baseline (empty shader):      "
                  << std::fixed << std::setprecision(4) << glBase.gpuTimeMs << " ms\n"
                  << "    OpenGL GPU time growth (10 -> 5000): "
                  << std::fixed << std::setprecision(1) << glGrowth << "x\n"
                  << "    Vulkan GPU time growth (10 -> 5000): "
                  << std::fixed << std::setprecision(1) << vkGrowth << "x\n\n";

        if (vkBase.success && rows.back().vkOk) {
            double vkHeavyVsBase = rows.back().vkTimeMs / (vkBase.gpuTimeMs > 0 ? vkBase.gpuTimeMs : 1.0);
            std::cout << "    Vulkan heavy(5000) mask=0 / baseline: "
                      << std::fixed << std::setprecision(2) << vkHeavyVsBase << "x\n";
            if (vkHeavyVsBase < 2.0) {
                std::cout << "    -> Vulkan PSO DCE eliminated the heavy loop entirely!\n";
            }
        }

        if (glBase.success && rows.back().glOk) {
            double glHeavyVsBase = rows.back().glTimeMs / (glBase.gpuTimeMs > 0 ? glBase.gpuTimeMs : 1.0);
            std::cout << "    OpenGL heavy(5000) mask=0 / baseline: "
                      << std::fixed << std::setprecision(2) << glHeavyVsBase << "x\n";
            if (glHeavyVsBase > 5.0) {
                std::cout << "    -> OpenGL still executes the heavy loop (no cross-stage DCE)\n";
            }
        }
        std::cout << "\n";
    }

    if (rowsCW.size() >= 2 && rowsCW.front().vkOk && rowsCW.back().vkOk) {
        double vkCWGrowth = rowsCW.back().vkTimeMs / (rowsCW.front().vkTimeMs > 0 ? rowsCW.front().vkTimeMs : 1.0);
        double glCWGrowth = rowsCW.back().glTimeMs / (rowsCW.front().glTimeMs > 0 ? rowsCW.front().glTimeMs : 1.0);

        std::cout << "  Analysis (Phase 2 – colorWriteMask=RGBA):\n"
                  << "    Vulkan GPU time growth (10 -> 5000): "
                  << std::fixed << std::setprecision(1) << vkCWGrowth << "x\n"
                  << "    OpenGL GPU time growth (10 -> 5000): "
                  << std::fixed << std::setprecision(1) << glCWGrowth << "x\n";

        // Compare mask=0 vs mask=RGBA for Vulkan at highest loop count
        if (!rows.empty() && rows.back().vkOk && rowsCW.back().vkOk && rows.back().vkTimeMs > 0) {
            double vkDCERatio = rowsCW.back().vkTimeMs / rows.back().vkTimeMs;
            std::cout << "    Vulkan mask=RGBA / mask=0 at loop=5000: "
                      << std::fixed << std::setprecision(1) << vkDCERatio << "x\n";
            if (vkDCERatio > 5.0) {
                std::cout << "    -> HUGE gap! Vulkan PSO DCE saved "
                          << std::fixed << std::setprecision(1)
                          << (1.0 - 1.0/vkDCERatio) * 100.0 << "% GPU time!\n";
            }
        }

        // Compare mask=0 vs mask=RGBA for OpenGL at highest loop count
        if (!rows.empty() && rows.back().glOk && rowsCW.back().glOk && rows.back().glTimeMs > 0) {
            double glDCERatio = rowsCW.back().glTimeMs / rows.back().glTimeMs;
            std::cout << "    OpenGL mask=RGBA / mask=0 at loop=5000: "
                      << std::fixed << std::setprecision(1) << glDCERatio << "x\n";
            if (glDCERatio < 1.5) {
                std::cout << "    -> OpenGL shows similar time with/without mask\n"
                          << "       (confirms: OpenGL cannot DCE based on runtime state)\n";
            }
        }
        std::cout << "\n";
    }

    // ── Final conclusion ───────────────────────────────────────────
    std::cout << "  >> CONCLUSION:\n";
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
            std::cout << "     Vulkan PSO cross-stage DCE is PROVEN:\n"
                      << "       - Vulkan mask=0 is dramatically faster than mask=RGBA\n"
                      << "         (compiler eliminated dead code when output is unused)\n"
                      << "       - OpenGL mask=0 is similar to mask=RGBA\n"
                      << "         (runtime state cannot trigger compile-time DCE)\n";
        } else if (vkDCEWorks) {
            std::cout << "     Vulkan PSO DCE optimization is effective.\n"
                      << "     OpenGL results are inconclusive.\n";
        } else {
            std::cout << "     Results are inconclusive. Driver may be doing\n"
                      << "     hardware-level optimization in both cases.\n";
        }
    }

    std::cout << "\n================================================================\n\n";

    return 0;
}
