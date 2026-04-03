#include "Runner/benchmark_runner.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("android_mesa_vulkan");

    test_support::expect_true(!plan.empty(), "mode has cases");
    test_support::expect_true(plan.front().driver_mode == "Mesa", "driver mode");

    return test_support::finish();
}
