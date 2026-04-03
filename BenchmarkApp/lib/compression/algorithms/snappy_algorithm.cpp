#include "compression/algorithms/compression_algorithm.h"

#include <stdexcept>
#include <snappy.h>

namespace benchmark {

class SnappyAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "snappy"; }
    std::string version() const override {
        return std::to_string(SNAPPY_MAJOR) + "." +
               std::to_string(SNAPPY_MINOR) + "." +
               std::to_string(SNAPPY_PATCHLEVEL);
    }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& /*level*/) override {
        std::string output;
        snappy::Compress(
            reinterpret_cast<const char*>(input.data()),
            input.size(), &output);
        std::vector<uint8_t> out(output.begin(), output.end());
        return {out, "snappy", compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        std::string input(payload.data.begin(), payload.data.end());
        std::string output;
        if (!snappy::Uncompress(input.data(), input.size(), &output)) {
            throw std::runtime_error("snappy decompression failed");
        }
        return std::vector<uint8_t>(output.begin(), output.end());
    }

    std::vector<std::string> supported_levels() const override { return {"0"}; }
};

// Self-registration
static const bool _reg_snappy = (CompressionRegistry::instance().add(
    "snappy", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<SnappyAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_snappy_algorithm() {
    return std::make_unique<SnappyAlgorithm>();
}

}  // namespace benchmark
