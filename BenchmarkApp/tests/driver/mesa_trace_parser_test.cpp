#include <iostream>
#include <vector>

#include "driver/mesa/mesa_trace_parser.h"
#include "test_support/assert.h"

int main() {
    // Normal two-line input
    {
        const auto events = benchmark::parse_mesa_trace_lines({
            "stage=frontend duration_us=12",
            "stage=backend duration_us=20",
        });

        test_support::expect_equal(events.size(), static_cast<size_t>(2), "event count");
        test_support::expect_equal(events[0].name, std::string("frontend"), "first stage name");
        test_support::expect_equal(events[0].duration_us, static_cast<int64_t>(12), "first duration_us");
        test_support::expect_equal(events[1].name, std::string("backend"), "second stage name");
        test_support::expect_equal(events[1].duration_us, static_cast<int64_t>(20), "second duration_us");
    }

    // Empty input returns empty
    {
        const auto events = benchmark::parse_mesa_trace_lines({});
        test_support::expect_equal(events.size(), static_cast<size_t>(0), "empty input returns empty");
    }

    // Malformed lines: missing stage=
    {
        const auto events = benchmark::parse_mesa_trace_lines({
            "duration_us=12",
        });
        test_support::expect_equal(events.size(), static_cast<size_t>(0),
            "missing stage= produces no event");
    }

    // Malformed lines: missing duration_us=
    {
        const auto events = benchmark::parse_mesa_trace_lines({
            "stage=frontend",
        });
        test_support::expect_equal(events.size(), static_cast<size_t>(0),
            "missing duration_us= produces no event");
    }

    // Single-line input
    {
        const auto events = benchmark::parse_mesa_trace_lines({
            "stage=optimizer duration_us=55",
        });
        test_support::expect_equal(events.size(), static_cast<size_t>(1), "single line count");
        test_support::expect_equal(events[0].name, std::string("optimizer"), "single line stage name");
        test_support::expect_equal(events[0].duration_us, static_cast<int64_t>(55), "single line duration");
    }

    std::cout << "mesa_trace_parser_test: all checks completed\n";
    return test_support::finish();
}
