#include "compression/algorithms/compression_algorithm.h"

#include <stdexcept>

namespace benchmark {

// Deflate-lite: two-pass compressor combining RLE and dictionary.
// Pass 1: RLE pass to collapse byte runs
// Pass 2: Dictionary pass on the RLE output for repeated sequences
// Format: header (8 bytes orig size) + 4 bytes intermediate size + dict-compressed(rle-compressed(data))

class DeflateLiteAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "deflate_lite"; }
    std::string version() const override { return "1.0.0"; }

    CompressedPayload compress(const std::vector<uint8_t>& input, const std::string& level) override {
        auto rle = make_rle_algorithm();
        auto dict = make_dictionary_algorithm();

        // Pass 1: RLE
        auto rle_result = rle->compress(input, level);

        // Pass 2: Dictionary on the RLE output (skip RLE header, use raw compressed bytes)
        auto dict_result = dict->compress(rle_result.data, level);

        CompressedPayload result;
        result.algorithm = name();
        result.data = std::move(dict_result.data);
        result.hash = compute_hash(input);
        return result;
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        auto rle = make_rle_algorithm();
        auto dict = make_dictionary_algorithm();

        // Reverse pass 2: dictionary decompress
        CompressedPayload dict_payload;
        dict_payload.data = payload.data;
        dict_payload.algorithm = "dictionary";
        auto rle_compressed = dict->decompress(dict_payload);

        // Reverse pass 1: RLE decompress
        CompressedPayload rle_payload;
        rle_payload.data = std::move(rle_compressed);
        rle_payload.algorithm = "rle";
        return rle->decompress(rle_payload);
    }

    std::vector<std::string> supported_levels() const override { return {"6"}; }
};

std::unique_ptr<CompressionAlgorithm> make_deflate_lite_algorithm() {
    return std::make_unique<DeflateLiteAlgorithm>();
}

}  // namespace benchmark
