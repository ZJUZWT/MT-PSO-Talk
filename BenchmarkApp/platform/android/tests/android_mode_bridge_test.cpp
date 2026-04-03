#include <string>

#include "runner/benchmark_runner.h"
#include "test_support/assert.h"

extern "C" const char* benchmark_default_mode();

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("android_native_opengl");

    test_support::expect_true(!plan.empty(), "android opengl mode available");
    test_support::expect_true(
        std::string(benchmark_default_mode()) == "android_native_opengl",
        "android default mode");

    return test_support::finish();
}
