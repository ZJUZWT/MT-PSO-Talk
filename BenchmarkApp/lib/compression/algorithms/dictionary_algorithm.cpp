#include "compression/algorithms/compression_algorithm.h"

#include <algorithm>
#include <cstring>

namespace benchmark {

// Simple LZ77-style sliding window dictionary compression.
// Format: stream of tokens, each token is:
//   [flags_byte]  -- bit 7: 1=match, 0=literal
//   If literal: [byte]
//   If match:   [offset_high] [offset_low] [length] (offset is 1-based backward)
// Window size: up to 4096 bytes back, match length 3..258.

static constexpr size_t kWindowSize = 4096;
static constexpr size_t kMinMatch = 3;
static constexpr size_t kMaxMatch = 258;

class DictionaryAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "dictionary"; }
    std::string version() const override { return "1.0.0"; }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& /*level*/) override {
        CompressedPayload result;
        result.algorithm = name();
        auto& out = result.data;
        out.reserve(input.size());

        const size_t n = input.size();
        size_t i = 0;

        while (i < n) {
            size_t best_offset = 0;
            size_t best_len = 0;

            // Search for longest match in window
            size_t window_start = (i > kWindowSize) ? (i - kWindowSize) : 0;
            for (size_t j = window_start; j < i; ++j) {
                size_t len = 0;
                size_t max_len = std::min(kMaxMatch, n - i);
                while (len < max_len && input[j + len] == input[i + len]) {
                    ++len;
                }
                if (len >= kMinMatch && len > best_len) {
                    best_len = len;
                    best_offset = i - j;
                }
            }

            if (best_len >= kMinMatch) {
                // Match token
                out.push_back(0x80);  // flags: match
                out.push_back(static_cast<uint8_t>((best_offset >> 8) & 0x0F));
                out.push_back(static_cast<uint8_t>(best_offset & 0xFF));
                out.push_back(static_cast<uint8_t>(best_len - kMinMatch));
                i += best_len;
            } else {
                // Literal token
                out.push_back(0x00);  // flags: literal
                out.push_back(input[i]);
                ++i;
            }
        }

        result.hash = compute_hash(input);
        return result;
    }

    std::vector<uint8_t> decompress(const CompressedPayload& payload) override {
        std::vector<uint8_t> out;
        const auto& data = payload.data;
        size_t i = 0;
        const size_t n = data.size();

        while (i < n) {
            uint8_t flags = data[i++];
            if (flags & 0x80) {
                // Match
                if (i + 2 >= n) break;
                size_t offset = (static_cast<size_t>(data[i] & 0x0F) << 8) |
                                 static_cast<size_t>(data[i+1]);
                size_t length = static_cast<size_t>(data[i+2]) + kMinMatch;
                i += 3;

                if (offset == 0 || offset > out.size()) break;
                size_t src = out.size() - offset;
                for (size_t k = 0; k < length; ++k) {
                    out.push_back(out[src + k]);
                }
            } else {
                // Literal
                if (i >= n) break;
                out.push_back(data[i++]);
            }
        }

        return out;
    }

    std::vector<std::string> supported_levels() const override { return {"3"}; }
};

std::unique_ptr<CompressionAlgorithm> make_dictionary_algorithm() {
    return std::make_unique<DictionaryAlgorithm>();
}

}  // namespace benchmark
