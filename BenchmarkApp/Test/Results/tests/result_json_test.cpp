#include <string>

#include "Results/schema/result_json.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::GraphicsResult result{};
    result.platform = "Android";
    result.api = "Vulkan";
    result.driver_mode = "Mesa";
    result.total_us = 42;

    const std::string json = benchmark::to_json(result);
    test_support::expect_true(
        json.find("\"platform\":\"Android\"") != std::string::npos,
        "platform serialized");
    test_support::expect_true(
        json.find("\"total_us\":42") != std::string::npos,
        "total serialized");
    return test_support::finish();
}
