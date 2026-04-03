#include "compression/algorithms/compression_algorithm.h"

#include <cstring>
#include <stdexcept>
#include <zlib.h>

namespace benchmark {

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

class ZlibAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "zlib"; }
    std::string version() const override { return zlibVersion(); }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        int lvl = std::stoi(level);
        uLong bound = compressBound(static_cast<uLong>(input.size()));

        std::vector<uint8_t> output;
        output.reserve(8 + bound);
        write_u64_le(output, static_cast<uint64_t>(input.size()));
        output.resize(8 + bound);

        uLongf dest_len = bound;
        int ret = compress2(
            output.data() + 8, &dest_len,
            input.data(), static_cast<uLong>(input.size()), lvl);
        if (ret != Z_OK) {
            throw std::runtime_error("zlib compress2 failed: " + std::to_string(ret));
        }
        output.resize(8 + dest_len);
        return {output, "zlib", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        if (payload.data.size() < 8) {
            throw std::runtime_error("zlib payload too small");
        }
        uint64_t original_size = read_u64_le(payload.data.data());
        std::vector<uint8_t> output(static_cast<size_t>(original_size));

        uLongf dest_len = static_cast<uLongf>(original_size);
        int ret = uncompress(
            output.data(), &dest_len,
            payload.data.data() + 8,
            static_cast<uLong>(payload.data.size() - 8));
        if (ret != Z_OK) {
            throw std::runtime_error("zlib uncompress failed: " + std::to_string(ret));
        }
        output.resize(static_cast<size_t>(dest_len));
        return output;
    }

    std::vector<std::string> supported_levels() const override { return {"1", "6", "9"}; }
};

// Self-registration
static const bool _reg_zlib = (CompressionRegistry::instance().add(
    "zlib", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<ZlibAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_zlib_algorithm() {
    return std::make_unique<ZlibAlgorithm>();
}

}  // namespace benchmark
