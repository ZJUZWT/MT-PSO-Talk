#include "compression/algorithms/compression_algorithm.h"

namespace benchmark {

// No-op compression: just copies data. Used as a baseline to measure
// framework overhead.
class NoopAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "noop"; }
    std::string version() const override { return "1.0.0"; }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& /*level*/) override {
        CompressedPayload result;
        result.algorithm = name();
        result.data = input;
        result.hash = compute_hash(input);
        return result;
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        return payload.data;
    }

    std::vector<std::string> supported_levels() const override { return {"0"}; }
};

// Self-registration
static const bool _reg_noop = (CompressionRegistry::instance().add(
    "noop", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<NoopAlgorithm>();
    }), true);

std::unique_ptr<CompressionAlgorithm> make_noop_algorithm() {
    return std::make_unique<NoopAlgorithm>();
}

}  // namespace benchmark
