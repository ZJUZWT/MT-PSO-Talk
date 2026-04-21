#pragma once

#include <string>

namespace benchmark {

struct AppleProductInfo {
    std::string marketing_name;
    std::string soc;
};

AppleProductInfo lookup_apple_product_info(const std::string& identifier);

}  // namespace benchmark
