#pragma once

#include <cstdint>
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

template<typename T>
inline void expect_equal(const T& actual, const T& expected, const std::string& label) {
    if (actual != expected) {
        std::cerr << "EXPECT_EQUAL failed: " << label
                  << " (actual=" << actual << ", expected=" << expected << ")\n";
        ++failure_count();
    }
}

inline void expect_greater_than(int64_t actual, int64_t threshold, const std::string& label) {
    if (actual <= threshold) {
        std::cerr << "EXPECT_GT failed: " << label
                  << " (actual=" << actual << ", threshold=" << threshold << ")\n";
        ++failure_count();
    }
}

inline void expect_less_than(int64_t actual, int64_t threshold, const std::string& label) {
    if (actual >= threshold) {
        std::cerr << "EXPECT_LT failed: " << label
                  << " (actual=" << actual << ", threshold=" << threshold << ")\n";
        ++failure_count();
    }
}

inline void expect_greater_than_double(double actual, double threshold, const std::string& label) {
    if (actual <= threshold) {
        std::cerr << "EXPECT_GT failed: " << label
                  << " (actual=" << actual << ", threshold=" << threshold << ")\n";
        ++failure_count();
    }
}

inline void expect_contains(const std::string& haystack, const std::string& needle, const std::string& label) {
    if (haystack.find(needle) == std::string::npos) {
        std::cerr << "EXPECT_CONTAINS failed: " << label
                  << " (needle=\"" << needle << "\" not found)\n";
        ++failure_count();
    }
}

inline void expect_not_contains(const std::string& haystack,
                                const std::string& needle,
                                const std::string& label) {
    if (haystack.find(needle) != std::string::npos) {
        std::cerr << "EXPECT_NOT_CONTAINS failed: " << label
                  << " (needle=\"" << needle << "\" unexpectedly found)\n";
        ++failure_count();
    }
}

inline int finish() {
    return failure_count() == 0 ? 0 : 1;
}

}  // namespace test_support
