#include <string>

#include "Results/exporters/json_exporter.h"
#include "TestSupport/assert.h"

int main() {
    benchmark::GraphicsResult result{};
    result.platform = "Android";
    result.api = "OpenGL";

    const auto json = benchmark::export_graphics_json(result);
    test_support::expect_true(json.find("OpenGL") != std::string::npos, "json export");

    return test_support::finish();
}
