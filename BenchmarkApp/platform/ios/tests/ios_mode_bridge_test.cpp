#include <string>

#include "runner/benchmark_runner.h"
#include "test_support/assert.h"

extern "C" const char* benchmark_default_ios_mode();

int main() {
    benchmark::BenchmarkRunner runner;
    const auto plan = runner.plan_for_mode("ios_native_metal");

    test_support::expect_true(!plan.empty(), "ios metal mode available");
    test_support::expect_true(
        std::string(benchmark_default_ios_mode()) == "ios_native_metal",
        "ios default mode");

    return test_support::finish();
}
