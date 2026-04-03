#include "compression/payload_generator.h"

#include <algorithm>
#include <cstdint>

namespace benchmark {
namespace {

// Simple deterministic PRNG (xorshift64) for reproducible payloads
class Xorshift64 {
public:
    explicit Xorshift64(uint64_t seed = 0x12345678DEADBEEFULL) : state_(seed) {}

    uint64_t next() {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 7;
        state_ ^= state_ << 17;
        return state_;
    }

    uint8_t next_byte() {
        return static_cast<uint8_t>(next() & 0xFF);
    }

private:
    uint64_t state_;
};

// PSO cache header structure (synthetic)
struct PsoCacheHeader {
    uint32_t magic = 0x50534F43;  // "PSOC"
    uint32_t version = 1;
    uint32_t entry_count = 0;
    uint32_t flags = 0;
    uint8_t platform_id[16] = {};
    uint8_t reserved[32] = {};
};

static_assert(sizeof(PsoCacheHeader) == 64, "header size");

}  // namespace

std::vector<uint8_t> generate_pso_cache_payload(int64_t target_size) {
    if (target_size <= 0) return {};

    std::vector<uint8_t> payload;
    payload.reserve(static_cast<size_t>(target_size));

    Xorshift64 rng;

    // Write PSO cache header
    PsoCacheHeader header;
    header.entry_count = static_cast<uint32_t>(target_size / 256);
    const auto* hdr_bytes = reinterpret_cast<const uint8_t*>(&header);
    for (size_t i = 0; i < sizeof(header) && payload.size() < static_cast<size_t>(target_size); ++i) {
        payload.push_back(hdr_bytes[i]);
    }

    // Generate repeating shader hash blocks (32-byte hashes) + state vectors
    // This creates patterns typical of real PSO caches: some repeated data,
    // some varying data, lots of zero padding.
    while (payload.size() < static_cast<size_t>(target_size)) {
        size_t remaining = static_cast<size_t>(target_size) - payload.size();

        // Pattern 1: shader hash block (32 bytes, semi-random, often repeated)
        if (remaining >= 32) {
            uint8_t hash_template = rng.next_byte() & 0x07;  // Only 8 unique templates
            for (int i = 0; i < 32 && payload.size() < static_cast<size_t>(target_size); ++i) {
                payload.push_back(static_cast<uint8_t>(hash_template * 31 + i * 7));
            }
        }

        remaining = static_cast<size_t>(target_size) - payload.size();

        // Pattern 2: state vector (64 bytes, mostly small values + zero padding)
        if (remaining >= 64) {
            // A few meaningful state bytes
            for (int i = 0; i < 8 && payload.size() < static_cast<size_t>(target_size); ++i) {
                payload.push_back(rng.next_byte() & 0x0F);  // Small values 0-15
            }
            // Zero padding
            for (int i = 0; i < 56 && payload.size() < static_cast<size_t>(target_size); ++i) {
                payload.push_back(0x00);
            }
        }

        remaining = static_cast<size_t>(target_size) - payload.size();

        // Pattern 3: pipeline descriptor block (128 bytes with repeated enum-like values)
        if (remaining >= 128) {
            // Repeated 4-byte enum values
            uint8_t enum_val = rng.next_byte() & 0x03;
            for (int i = 0; i < 128 && payload.size() < static_cast<size_t>(target_size); ++i) {
                if (i % 4 == 0) {
                    payload.push_back(enum_val);
                } else {
                    payload.push_back(0x00);
                }
            }
        }

        remaining = static_cast<size_t>(target_size) - payload.size();

        // Fill any small remainder with padding
        if (remaining > 0 && remaining < 32) {
            for (size_t i = 0; i < remaining; ++i) {
                payload.push_back(0x00);
            }
        }
    }

    payload.resize(static_cast<size_t>(target_size));
    return payload;
}

std::vector<uint8_t> generate_payload_with_compressibility(int64_t target_size, double compressibility) {
    if (target_size <= 0) return {};

    // Clamp to [0, 1]
    compressibility = std::max(0.0, std::min(1.0, compressibility));

    std::vector<uint8_t> payload;
    payload.reserve(static_cast<size_t>(target_size));

    Xorshift64 rng;

    // Ratio of compressible (repeated/zero) vs random bytes
    const size_t total = static_cast<size_t>(target_size);

    for (size_t i = 0; i < total; ++i) {
        // Use a deterministic threshold to decide compressible vs random
        double threshold = static_cast<double>(i % 256) / 256.0;
        if (threshold < compressibility) {
            // Compressible: zero or repeated pattern
            payload.push_back(0x00);
        } else {
            // Random byte
            payload.push_back(rng.next_byte());
        }
    }

    return payload;
}

}  // namespace benchmark
