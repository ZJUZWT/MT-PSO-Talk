#include "compression/algorithms/compression_algorithm.h"

namespace benchmark {

// Run-length encoding: encodes runs of identical bytes.
// Format: series of chunks. Each chunk:
//   If a run of 3+ identical bytes: [0xFF] [count_high] [count_low] [byte]
//   Otherwise: [count] [byte0] [byte1] ... (literal run, count < 255)
// count for runs is 16-bit big-endian to support runs up to 65535.

class RleAlgorithm : public CompressionAlgorithm {
public:
    std::string name() const override { return "rle"; }
    std::string version() const override { return "1.0.0"; }

    CompressedPayload compress(const std::vector<uint8_t>& input,
                               const std::string& /*level*/) override {
        CompressedPayload result;
        result.algorithm = name();

        std::vector<uint8_t>& out = result.data;
        out.reserve(input.size());

        size_t i = 0;
        const size_t n = input.size();

        while (i < n) {
            // Count run length of identical bytes
            size_t run_start = i;
            uint8_t byte = input[i];
            while (i < n && input[i] == byte && (i - run_start) < 65535) {
                ++i;
            }
            size_t run_len = i - run_start;

            if (run_len >= 3) {
                // Encode as run: marker(0xFF) + 2-byte length + byte value
                out.push_back(0xFF);
                out.push_back(static_cast<uint8_t>((run_len >> 8) & 0xFF));
                out.push_back(static_cast<uint8_t>(run_len & 0xFF));
                out.push_back(byte);
            } else {
                // Collect literal bytes until we hit a run of 3+
                // Back up: we already consumed run_len bytes of 'byte'
                size_t lit_start = run_start;
                i = run_start;

                std::vector<uint8_t> literals;
                while (i < n && literals.size() < 254) {
                    // Check if a run of 3+ starts here
                    if (i + 2 < n && input[i] == input[i+1] && input[i] == input[i+2]) {
                        break;
                    }
                    literals.push_back(input[i]);
                    ++i;
                }
                if (literals.empty()) {
                    // Edge case: we're at a run but it was < 3 initially;
                    // just emit as literals
                    for (size_t j = 0; j < run_len && j < 254; ++j) {
                        literals.push_back(byte);
                    }
                    i = run_start + literals.size();
                }
                out.push_back(static_cast<uint8_t>(literals.size()));
                out.insert(out.end(), literals.begin(), literals.end());
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
            if (data[i] == 0xFF) {
                // Run encoding
                if (i + 3 >= n) break;
                size_t run_len = (static_cast<size_t>(data[i+1]) << 8) |
                                  static_cast<size_t>(data[i+2]);
                uint8_t byte = data[i+3];
                out.insert(out.end(), run_len, byte);
                i += 4;
            } else {
                // Literal run
                size_t count = data[i];
                ++i;
                if (i + count > n) count = n - i;
                out.insert(out.end(), data.begin() + i,
                           data.begin() + i + count);
                i += count;
            }
        }

        return out;
    }

    std::vector<std::string> supported_levels() const override { return {"1"}; }
};

std::unique_ptr<CompressionAlgorithm> make_rle_algorithm() {
    return std::make_unique<RleAlgorithm>();
}

}  // namespace benchmark
