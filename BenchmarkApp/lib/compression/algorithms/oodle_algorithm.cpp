#include "compression/algorithms/compression_algorithm.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#if BENCHMARK_HAS_OODLE
#include <oodle2.h>
#endif

namespace benchmark {

#if BENCHMARK_HAS_OODLE

class OodleAlgorithm : public CompressionAlgorithm {
public:
    OodleAlgorithm(OodleLZ_Compressor compressor, const std::string& algo_name,
                   std::vector<std::string> levels)
        : compressor_(compressor), name_(algo_name), levels_(std::move(levels)) {}

    std::string name() const override { return name_; }
    std::string version() const override { return "2.9.12"; }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& level) override {
        OodleLZ_CompressionLevel oodleLevel = parse_level(level);

        OO_SINTa bound = OodleLZ_GetCompressedBufferSizeNeeded(
            compressor_, static_cast<OO_SINTa>(input.size()));
        // +8 bytes header to store original size for decompression
        std::vector<uint8_t> output(static_cast<size_t>(bound) + 8);

        // Store original size in first 8 bytes (little-endian native)
        int64_t raw_size = static_cast<int64_t>(input.size());
        std::memcpy(output.data(), &raw_size, 8);

        OO_SINTa compressed_size = OodleLZ_Compress(
            compressor_,
            input.data(), static_cast<OO_SINTa>(input.size()),
            output.data() + 8,
            oodleLevel,
            nullptr,   // pOptions
            nullptr,   // dictionaryBase
            nullptr,   // lrm
            nullptr,   // scratchMem
            0);        // scratchSize

        if (compressed_size <= 0) {
            throw std::runtime_error("OodleLZ_Compress failed for " + name_);
        }

        output.resize(8 + static_cast<size_t>(compressed_size));
        return {output, name_, compute_hash(input)};
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        if (payload.data.size() < 8) {
            throw std::runtime_error("Oodle compressed payload too small");
        }

        // Read original size from first 8 bytes
        int64_t raw_size = 0;
        std::memcpy(&raw_size, payload.data.data(), 8);

        std::vector<uint8_t> output(static_cast<size_t>(raw_size));

        OO_SINTa result = OodleLZ_Decompress(
            payload.data.data() + 8,
            static_cast<OO_SINTa>(payload.data.size() - 8),
            output.data(),
            static_cast<OO_SINTa>(raw_size),
            OodleLZ_FuzzSafe_Yes,
            OodleLZ_CheckCRC_No,
            OodleLZ_Verbosity_None,
            nullptr, 0,       // decBufBase, decBufSize
            nullptr, nullptr,  // fpCallback, callbackUserData
            nullptr, 0,       // decoderMemory, decoderMemorySize
            OodleLZ_Decode_Unthreaded);

        if (result != static_cast<OO_SINTa>(raw_size)) {
            throw std::runtime_error("OodleLZ_Decompress failed for " + name_);
        }

        return output;
    }

    std::vector<std::string> supported_levels() const override { return levels_; }

private:
    OodleLZ_Compressor compressor_;
    std::string name_;
    std::vector<std::string> levels_;

    static OodleLZ_CompressionLevel parse_level(const std::string& level) {
        int lvl = std::stoi(level);
        if (lvl <= 1) return OodleLZ_CompressionLevel_SuperFast;
        if (lvl <= 3) return OodleLZ_CompressionLevel_Fast;
        if (lvl <= 4) return OodleLZ_CompressionLevel_Normal;
        if (lvl <= 6) return static_cast<OodleLZ_CompressionLevel>(6);  // Optimal2
        return static_cast<OodleLZ_CompressionLevel>(9);                // Optimal5
    }
};

// Self-registration for all Oodle variants
static const bool _reg_oodle_kraken = (CompressionRegistry::instance().add(
    "oodle_kraken", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<OodleAlgorithm>(
            static_cast<OodleLZ_Compressor>(8), "oodle_kraken",
            std::vector<std::string>{"1", "4"});
    }), true);

static const bool _reg_oodle_leviathan = (CompressionRegistry::instance().add(
    "oodle_leviathan", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<OodleAlgorithm>(
            static_cast<OodleLZ_Compressor>(13), "oodle_leviathan",
            std::vector<std::string>{"1", "4"});
    }), true);

static const bool _reg_oodle_mermaid = (CompressionRegistry::instance().add(
    "oodle_mermaid", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<OodleAlgorithm>(
            static_cast<OodleLZ_Compressor>(9), "oodle_mermaid",
            std::vector<std::string>{"1", "4"});
    }), true);

static const bool _reg_oodle_selkie = (CompressionRegistry::instance().add(
    "oodle_selkie", []() -> std::unique_ptr<CompressionAlgorithm> {
        return std::make_unique<OodleAlgorithm>(
            static_cast<OodleLZ_Compressor>(11), "oodle_selkie",
            std::vector<std::string>{"1", "4"});
    }), true);

std::unique_ptr<CompressionAlgorithm> make_oodle_kraken_algorithm() {
    return std::make_unique<OodleAlgorithm>(
        static_cast<OodleLZ_Compressor>(8), "oodle_kraken",
        std::vector<std::string>{"1", "4"});
}

std::unique_ptr<CompressionAlgorithm> make_oodle_leviathan_algorithm() {
    return std::make_unique<OodleAlgorithm>(
        static_cast<OodleLZ_Compressor>(13), "oodle_leviathan",
        std::vector<std::string>{"1", "4"});
}

std::unique_ptr<CompressionAlgorithm> make_oodle_mermaid_algorithm() {
    return std::make_unique<OodleAlgorithm>(
        static_cast<OodleLZ_Compressor>(9), "oodle_mermaid",
        std::vector<std::string>{"1", "4"});
}

std::unique_ptr<CompressionAlgorithm> make_oodle_selkie_algorithm() {
    return std::make_unique<OodleAlgorithm>(
        static_cast<OodleLZ_Compressor>(11), "oodle_selkie",
        std::vector<std::string>{"1", "4"});
}

#else

class OodleUnavailableAlgorithm : public CompressionAlgorithm {
public:
    explicit OodleUnavailableAlgorithm(std::string algorithm_name)
        : name_(std::move(algorithm_name)) {}

    std::string name() const override { return name_; }
    std::string version() const override { return "2.9.12"; }

    CompressedPayload compress(const std::vector<uint8_t>&, const std::string&) override {
        throw std::runtime_error(name_ + " is unavailable in this build");
    }

    std::vector<uint8_t> decompress(const CompressedPayload&) override {
        throw std::runtime_error(name_ + " is unavailable in this build");
    }

    std::vector<std::string> supported_levels() const override {
        return {"1", "4"};
    }

private:
    std::string name_;
};

std::unique_ptr<CompressionAlgorithm> make_oodle_kraken_algorithm() {
    return std::make_unique<OodleUnavailableAlgorithm>("oodle_kraken");
}

std::unique_ptr<CompressionAlgorithm> make_oodle_leviathan_algorithm() {
    return std::make_unique<OodleUnavailableAlgorithm>("oodle_leviathan");
}

std::unique_ptr<CompressionAlgorithm> make_oodle_mermaid_algorithm() {
    return std::make_unique<OodleUnavailableAlgorithm>("oodle_mermaid");
}

std::unique_ptr<CompressionAlgorithm> make_oodle_selkie_algorithm() {
    return std::make_unique<OodleUnavailableAlgorithm>("oodle_selkie");
}

#endif

}  // namespace benchmark
