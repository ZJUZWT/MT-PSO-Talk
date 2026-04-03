#include "compression/algorithms/compression_algorithm.h"

#include <stdexcept>
#include <zstd.h>

namespace benchmark {

class ZstdAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "zstd"; }
    std::string version() const override { return ZSTD_versionString(); }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        int lvl = std::stoi(level);
        size_t bound = ZSTD_compressBound(input.size());
        std::vector<uint8_t> output(bound);
        size_t compressed_size = ZSTD_compress(
            output.data(), bound, input.data(), input.size(), lvl);
        if (ZSTD_isError(compressed_size)) {
            throw std::runtime_error(
                std::string("ZSTD_compress failed: ") + ZSTD_getErrorName(compressed_size));
        }
        output.resize(compressed_size);
        return {output, "zstd", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        unsigned long long decompressed_size =
            ZSTD_getFrameContentSize(payload.data.data(), payload.data.size());
        if (decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN ||
            decompressed_size == ZSTD_CONTENTSIZE_ERROR) {
            throw std::runtime_error("ZSTD_getFrameContentSize failed");
        }
        std::vector<uint8_t> output(static_cast<size_t>(decompressed_size));
        size_t result = ZSTD_decompress(
            output.data(), output.size(), payload.data.data(), payload.data.size());
        if (ZSTD_isError(result)) {
            throw std::runtime_error(
                std::string("ZSTD_decompress failed: ") + ZSTD_getErrorName(result));
        }
        output.resize(result);
        return output;
    }

    std::vector<std::string> supported_levels() const override { return {"1", "3", "9", "19"}; }
};

// Self-registration
static const bool _reg_zstd = (CompressionRegistry::instance().add(
    "zstd", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<ZstdAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_zstd_algorithm() {
    return std::make_unique<ZstdAlgorithm>();
}

}  // namespace benchmark
