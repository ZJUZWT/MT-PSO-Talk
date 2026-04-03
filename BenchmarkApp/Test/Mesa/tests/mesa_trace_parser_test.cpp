#include <vector>

#include "Mesa/adapters/mesa_trace_parser.h"
#include "TestSupport/assert.h"

int main() {
    const auto events = benchmark::parse_mesa_trace_lines({
        "stage=frontend duration_us=12",
        "stage=backend duration_us=20",
    });

    test_support::expect_true(events.size() == 2, "event count");
    test_support::expect_true(events.front().name == "frontend", "stage name");

    return test_support::finish();
}
