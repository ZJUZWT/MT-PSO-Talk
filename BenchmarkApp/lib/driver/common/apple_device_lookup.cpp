#include "driver/common/apple_device_lookup.h"

#include <array>
#include <string_view>

namespace benchmark {
namespace {

struct AppleProductMapping {
    std::string_view identifier;
    std::string_view marketing_name;
    std::string_view soc;
};

constexpr std::array<AppleProductMapping, 26> kAppleProductMappings = {{
    {"iPad13,1", "iPad Air (4th generation)", "Apple A14 Bionic"},
    {"iPad13,2", "iPad Air (4th generation)", "Apple A14 Bionic"},
    {"iPad13,4", "iPad Pro 11-inch (3rd generation)", "Apple M1"},
    {"iPad13,5", "iPad Pro 11-inch (3rd generation)", "Apple M1"},
    {"iPad13,6", "iPad Pro 11-inch (3rd generation)", "Apple M1"},
    {"iPad13,7", "iPad Pro 11-inch (3rd generation)", "Apple M1"},
    {"iPad13,8", "iPad Pro 12.9-inch (5th generation)", "Apple M1"},
    {"iPad13,9", "iPad Pro 12.9-inch (5th generation)", "Apple M1"},
    {"iPad13,10", "iPad Pro 12.9-inch (5th generation)", "Apple M1"},
    {"iPad13,11", "iPad Pro 12.9-inch (5th generation)", "Apple M1"},
    {"iPad13,16", "iPad Air (5th generation)", "Apple M1"},
    {"iPad13,17", "iPad Air (5th generation)", "Apple M1"},
    {"iPad14,1", "iPad mini (6th generation)", "Apple A15 Bionic"},
    {"iPad14,2", "iPad mini (6th generation)", "Apple A15 Bionic"},
    {"iPad14,3", "iPad Pro 11-inch (4th generation)", "Apple M2"},
    {"iPad14,4", "iPad Pro 11-inch (4th generation)", "Apple M2"},
    {"iPad14,5", "iPad Pro 12.9-inch (6th generation)", "Apple M2"},
    {"iPad14,6", "iPad Pro 12.9-inch (6th generation)", "Apple M2"},
    {"iPad14,8", "iPad Air 11-inch (M2)", "Apple M2"},
    {"iPad14,9", "iPad Air 11-inch (M2)", "Apple M2"},
    {"iPad14,10", "iPad Air 13-inch (M2)", "Apple M2"},
    {"iPad14,11", "iPad Air 13-inch (M2)", "Apple M2"},
    {"iPad16,3", "iPad Pro 11-inch (M4)", "Apple M4"},
    {"iPad16,4", "iPad Pro 11-inch (M4)", "Apple M4"},
    {"iPad16,5", "iPad Pro 13-inch (M4)", "Apple M4"},
    {"iPad16,6", "iPad Pro 13-inch (M4)", "Apple M4"},
}};

constexpr std::array<AppleProductMapping, 13> kApplePhoneMappings = {{
    {"iPhone14,2", "iPhone 13 Pro", "Apple A15 Bionic"},
    {"iPhone14,3", "iPhone 13 Pro Max", "Apple A15 Bionic"},
    {"iPhone14,4", "iPhone 13 mini", "Apple A15 Bionic"},
    {"iPhone14,5", "iPhone 13", "Apple A15 Bionic"},
    {"iPhone14,6", "iPhone SE (3rd generation)", "Apple A15 Bionic"},
    {"iPhone14,7", "iPhone 14", "Apple A15 Bionic"},
    {"iPhone14,8", "iPhone 14 Plus", "Apple A15 Bionic"},
    {"iPhone15,2", "iPhone 14 Pro", "Apple A16 Bionic"},
    {"iPhone15,3", "iPhone 14 Pro Max", "Apple A16 Bionic"},
    {"iPhone15,4", "iPhone 15", "Apple A16 Bionic"},
    {"iPhone15,5", "iPhone 15 Plus", "Apple A16 Bionic"},
    {"iPhone16,1", "iPhone 15 Pro", "Apple A17 Pro"},
    {"iPhone16,2", "iPhone 15 Pro Max", "Apple A17 Pro"},
}};

template <size_t N>
AppleProductInfo find_mapping(const std::array<AppleProductMapping, N>& mappings, const std::string& identifier) {
    for (const auto& mapping : mappings) {
        if (mapping.identifier == identifier) {
            return {std::string(mapping.marketing_name), std::string(mapping.soc)};
        }
    }
    return {};
}

}  // namespace

AppleProductInfo lookup_apple_product_info(const std::string& identifier) {
    if (identifier.empty()) {
        return {};
    }

    AppleProductInfo info = find_mapping(kAppleProductMappings, identifier);
    if (!info.marketing_name.empty() || !info.soc.empty()) {
        return info;
    }

    return find_mapping(kApplePhoneMappings, identifier);
}

}  // namespace benchmark
