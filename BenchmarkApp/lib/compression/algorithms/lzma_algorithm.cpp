#include "compression/algorithms/compression_algorithm.h"

#ifdef HAVE_LZMA

#include <cstring>
#include <stdexcept>
#include <lzma.h>

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

class LzmaAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "lzma"; }
    std::string version() const override { return lzma_version_string(); }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        uint32_t preset = static_cast<uint32_t>(std::stoi(level));
        size_t bound = lzma_stream_buffer_bound(input.size());

        std::vector<uint8_t> output;
        output.reserve(8 + bound);
        write_u64_le(output, static_cast<uint64_t>(input.size()));
        output.resize(8 + bound);

        size_t out_pos = 0;
        lzma_ret ret = lzma_easy_buffer_encode(
            preset, LZMA_CHECK_CRC64,
            nullptr,
            input.data(), input.size(),
            output.data() + 8, &out_pos, bound);
        if (ret != LZMA_OK) {
            throw std::runtime_error("lzma_easy_buffer_encode failed: " + std::to_string(ret));
        }
        output.resize(8 + out_pos);
        return {output, "lzma", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        if (payload.data.size() < 8) {
            throw std::runtime_error("lzma payload too small");
        }
        uint64_t original_size = read_u64_le(payload.data.data());
        std::vector<uint8_t> output(static_cast<size_t>(original_size));

        uint64_t memlimit = UINT64_MAX;
        size_t in_pos = 0;
        size_t out_pos = 0;
        lzma_ret ret = lzma_stream_buffer_decode(
            &memlimit, 0, nullptr,
            payload.data.data() + 8, &in_pos, payload.data.size() - 8,
            output.data(), &out_pos, output.size());
        if (ret != LZMA_OK) {
            throw std::runtime_error("lzma_stream_buffer_decode failed: " + std::to_string(ret));
        }
        output.resize(out_pos);
        return output;
    }

    std::vector<std::string> supported_levels() const override { return {"1", "6", "9"}; }
};

// Self-registration
static const bool _reg_lzma = (CompressionRegistry::instance().add(
    "lzma", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<LzmaAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_lzma_algorithm() {
    return std::make_unique<LzmaAlgorithm>();
}

}  // namespace benchmark

#endif  // HAVE_LZMA
