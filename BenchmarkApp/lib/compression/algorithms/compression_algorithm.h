#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "framework/registry.h"

namespace benchmark {

struct CompressedPayload {
    std::vector<uint8_t> data;
    std::string algorithm;
    std::string hash;  // FNV-1a checksum as 16-char hex string
};

// Abstract compression algorithm interface
class CompressionAlgorithm {
public:
    virtual ~CompressionAlgorithm() = default;
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual CompressedPayload compress(const std::vector<uint8_t>& input, const std::string& level) = 0;
    virtual std::vector<uint8_t> decompress(const CompressedPayload& payload) = 0;
    virtual std::vector<std::string> supported_levels() const = 0;
};

using CompressionRegistry = Registry<CompressionAlgorithm>;

// Built-in toy algorithms (kept for reference)
std::unique_ptr<CompressionAlgorithm> make_rle_algorithm();           // Run-length encoding (LZ4-like fast)
std::unique_ptr<CompressionAlgorithm> make_dictionary_algorithm();    // Sliding-window dictionary (zstd-like)
std::unique_ptr<CompressionAlgorithm> make_deflate_lite_algorithm();  // Two-pass RLE + dictionary (zlib-like)
std::unique_ptr<CompressionAlgorithm> make_noop_algorithm();          // No compression baseline

// Real compression libraries
std::unique_ptr<CompressionAlgorithm> make_zstd_algorithm();
std::unique_ptr<CompressionAlgorithm> make_lz4_algorithm();
std::unique_ptr<CompressionAlgorithm> make_zlib_algorithm();
std::unique_ptr<CompressionAlgorithm> make_snappy_algorithm();
std::unique_ptr<CompressionAlgorithm> make_brotli_algorithm();
#ifdef HAVE_LZMA
std::unique_ptr<CompressionAlgorithm> make_lzma_algorithm();
#endif

// Oodle algorithms (from UE SDK)
std::unique_ptr<CompressionAlgorithm> make_oodle_kraken_algorithm();
std::unique_ptr<CompressionAlgorithm> make_oodle_leviathan_algorithm();
std::unique_ptr<CompressionAlgorithm> make_oodle_mermaid_algorithm();
std::unique_ptr<CompressionAlgorithm> make_oodle_selkie_algorithm();

// FNV-1a hash producing a 16-char hex string
std::string compute_hash(const std::vector<uint8_t>& data);

}  // namespace benchmark
