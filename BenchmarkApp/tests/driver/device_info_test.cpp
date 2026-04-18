#include "driver/common/apple_device_lookup.h"
#include "driver/common/device_info.h"
#include "test_support/assert.h"

#include <iostream>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

int main() {
    {
        const auto mini6 = benchmark::lookup_apple_product_info("iPad14,1");
        test_support::expect_equal(mini6.marketing_name, std::string("iPad mini (6th generation)"),
                                   "iPad14,1 marketing name");
        test_support::expect_equal(mini6.soc, std::string("Apple A15 Bionic"),
                                   "iPad14,1 soc");
    }

    {
        const auto ipadProM2 = benchmark::lookup_apple_product_info("iPad14,3");
        test_support::expect_equal(ipadProM2.marketing_name,
                                   std::string("iPad Pro 11-inch (4th generation)"),
                                   "iPad14,3 marketing name");
        test_support::expect_equal(ipadProM2.soc, std::string("Apple M2"),
                                   "iPad14,3 soc");
    }

    {
        const auto unknown = benchmark::lookup_apple_product_info("iPad99,9");
        test_support::expect_true(unknown.marketing_name.empty(), "unknown identifier has empty marketing name");
        test_support::expect_true(unknown.soc.empty(), "unknown identifier has empty soc");
    }

    const auto info = benchmark::query_device_info();

    test_support::expect_true(!info.device_model.empty(), "device_model populated");
    test_support::expect_true(!info.soc.empty(), "soc populated");
    test_support::expect_true(!info.os_version.empty(), "os_version populated");

#if defined(__APPLE__) && !TARGET_OS_IPHONE
    test_support::expect_true(info.device_model != "Mac", "macOS device_model should be specific");
    test_support::expect_true(info.soc != "Apple Silicon", "macOS soc should be specific");
    test_support::expect_contains(info.os_version, "macOS", "macOS os_version label");
#elif defined(__APPLE__) && TARGET_OS_IPHONE
    test_support::expect_true(info.device_model != "iOS Device", "iOS device_model should be specific");
    test_support::expect_true(info.soc != "Apple Silicon", "iOS soc should be specific");
    test_support::expect_contains(info.os_version, "iOS", "iOS os_version label");
#endif

    std::cout << "device_info_test: all checks completed\n";
    return test_support::finish();
}
