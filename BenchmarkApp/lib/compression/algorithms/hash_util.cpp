#include "compression/algorithms/compression_algorithm.h"

#include <cstdio>

namespace benchmark {

std::string compute_hash(const std::vector<uint8_t>& data) {
    // FNV-1a 64-bit hash
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (const auto byte : data) {
        hash ^= static_cast<uint64_t>(byte);
        hash *= 0x100000001b3ULL;
    }
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx",
                  static_cast<unsigned long long>(hash));
    return std::string(buf, 16);
}

}  // namespace benchmark
