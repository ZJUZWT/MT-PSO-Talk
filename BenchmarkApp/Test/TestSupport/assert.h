#pragma once

#include <iostream>
#include <string>

namespace test_support {

inline int& failure_count() {
    static int count = 0;
    return count;
}

inline void expect_true(bool value, const std::string& label) {
    if (!value) {
        std::cerr << "EXPECT_TRUE failed: " << label << '\n';
        ++failure_count();
    }
}

inline int finish() {
    return failure_count() == 0 ? 0 : 1;
}

}  // namespace test_support
