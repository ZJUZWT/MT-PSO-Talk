#include "compression/algorithms/compression_algorithm.h"

#include <cstring>
#include <stdexcept>
#include <lz4.h>
#include <lz4hc.h>

namespace benchmark {

// LZ4 doesn't store original size, so we prepend it as 8 bytes (little-endian).
static void write_u64_le(std::vector<uint8_t>& out, uint64_t val) {
    for (int i = 0; i < 8; ++i) {
        out.push_back(static_cast<uint8_t>(val & 0xFF));
        val >>= 8;
    }
}

static uint64_t read_u64_le(const uint8_t* p) {
    uint64_t val = 0;
    for (int i = 7; i >= 0; --i) {
        val = (val << 8) | p[i];
    }
    return val;
}

class Lz4Algorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "lz4"; }
    std::string version() const override {
        int ver = LZ4_versionNumber();
        return std::to_string(ver / 10000) + "." +
               std::to_string((ver / 100) % 100) + "." +
               std::to_string(ver % 100);
    }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        int lvl = std::stoi(level);
        int src_size = static_cast<int>(input.size());
        int bound = LZ4_compressBound(src_size);

        std::vector<uint8_t> output;
        output.reserve(8 + bound);
        write_u64_le(output, static_cast<uint64_t>(input.size()));
        output.resize(8 + bound);

        int compressed_size;
        if (lvl >= 9) {
            compressed_size = LZ4_compress_HC(
                reinterpret_cast<const char*>(input.data()),
                reinterpret_cast<char*>(output.data() + 8),
                src_size, bound, lvl);
        } else {
            compressed_size = LZ4_compress_default(
                reinterpret_cast<const char*>(input.data()),
                reinterpret_cast<char*>(output.data() + 8),
                src_size, bound);
        }
        if (compressed_size <= 0) {
            throw std::runtime_error("LZ4 compression failed");
        }
        output.resize(8 + compressed_size);
        return {output, "lz4", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        if (payload.data.size() < 8) {
            throw std::runtime_error("LZ4 payload too small");
        }
        uint64_t original_size = read_u64_le(payload.data.data());
        std::vector<uint8_t> output(static_cast<size_t>(original_size));
        int result = LZ4_decompress_safe(
            reinterpret_cast<const char*>(payload.data.data() + 8),
            reinterpret_cast<char*>(output.data()),
            static_cast<int>(payload.data.size() - 8),
            static_cast<int>(original_size));
        if (result < 0) {
            throw std::runtime_error("LZ4 decompression failed");
        }
        output.resize(static_cast<size_t>(result));
        return output;
    }

    std::vector<std::string> supported_levels() const override { return {"1", "9"}; }
};

// Self-registration
static const bool _reg_lz4 = (CompressionRegistry::instance().add(
    "lz4", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<Lz4Algorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_lz4_algorithm() {
    return std::make_unique<Lz4Algorithm>();
}

}  // namespace benchmark
