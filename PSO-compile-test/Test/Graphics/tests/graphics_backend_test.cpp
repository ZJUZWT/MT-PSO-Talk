#include "Graphics/common/graphics_backend.h"
#include "TestSupport/assert.h"

int main() {
    const auto summary =
        benchmark::make_stub_native_backend().run_compile_case("S1_BasePBR");

    test_support::expect_true(summary.total_us >= 0, "timing emitted");
    test_support::expect_true(summary.api == "Stub", "api name");

    return test_support::finish();
}
