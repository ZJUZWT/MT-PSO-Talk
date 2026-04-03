#include "compression/algorithms/compression_algorithm.h"

#include <cstring>
#include <stdexcept>
#include <brotli/encode.h>
#include <brotli/decode.h>

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

class BrotliAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "brotli"; }
    std::string version() const override {
        return std::to_string(BrotliEncoderVersion() >> 24) + "." +
               std::to_string((BrotliEncoderVersion() >> 12) & 0xFFF) + "." +
               std::to_string(BrotliEncoderVersion() & 0xFFF);
    }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        int quality = std::stoi(level);
        size_t max_output = BrotliEncoderMaxCompressedSize(input.size());
        if (max_output == 0) {
            max_output = input.size() + 1024;  // fallback for empty input
        }

        std::vector<uint8_t> output;
        output.reserve(8 + max_output);
        write_u64_le(output, static_cast<uint64_t>(input.size()));
        output.resize(8 + max_output);

        size_t encoded_size = max_output;
        BROTLI_BOOL ok = BrotliEncoderCompress(
            quality, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE,
            input.size(), input.data(),
            &encoded_size, output.data() + 8);
        if (!ok) {
            throw std::runtime_error("BrotliEncoderCompress failed");
        }
        output.resize(8 + encoded_size);
        return {output, "brotli", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        if (payload.data.size() < 8) {
            throw std::runtime_error("brotli payload too small");
        }
        uint64_t original_size = read_u64_le(payload.data.data());
        std::vector<uint8_t> output(static_cast<size_t>(original_size));

        size_t decoded_size = output.size();
        BrotliDecoderResult result = BrotliDecoderDecompress(
            payload.data.size() - 8,
            payload.data.data() + 8,
            &decoded_size,
            output.data());
        if (result != BROTLI_DECODER_RESULT_SUCCESS) {
            throw std::runtime_error("BrotliDecoderDecompress failed");
        }
        output.resize(decoded_size);
        return output;
    }

    std::vector<std::string> supported_levels() const override { return {"1", "6", "11"}; }
};

// Self-registration
static const bool _reg_brotli = (CompressionRegistry::instance().add(
    "brotli", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<BrotliAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_brotli_algorithm() {
    return std::make_unique<BrotliAlgorithm>();
}

}  // namespace benchmark
