#include "compression/algorithms/compression_algorithm.h"
#include "compression/payload_generator.h"
#include "test_support/assert.h"

#include <iostream>
#include <memory>

namespace {

void test_roundtrip(benchmark::CompressionAlgorithm& algo, const std::string& label,
                    const std::vector<uint8_t>& input, const std::string& level = "default") {
    auto compressed = algo.compress(input, level);
    auto decompressed = algo.decompress(compressed);

    test_support::expect_true(
        decompressed.size() == input.size(),
        label + " roundtrip size (" + std::to_string(decompressed.size()) +
        " vs " + std::to_string(input.size()) + ")");

    bool data_match = (decompressed == input);
    test_support::expect_true(data_match, label + " roundtrip data match");

    // Verify hash
    std::string input_hash = benchmark::compute_hash(input);
    test_support::expect_true(compressed.hash == input_hash, label + " hash matches input");
    test_support::expect_true(compressed.hash.size() == 16, label + " hash is 16 chars");
}

}  // namespace

int main() {
    // Test compute_hash
    {
        std::vector<uint8_t> data1 = {1, 2, 3, 4, 5};
        std::vector<uint8_t> data2 = {1, 2, 3, 4, 6};
        std::vector<uint8_t> empty = {};

        auto h1 = benchmark::compute_hash(data1);
        auto h2 = benchmark::compute_hash(data2);
        auto h3 = benchmark::compute_hash(empty);
        auto h1_again = benchmark::compute_hash(data1);

        test_support::expect_true(h1.size() == 16, "hash length 16");
        test_support::expect_true(h1 != h2, "different data different hash");
        test_support::expect_true(h1 == h1_again, "same data same hash");
        test_support::expect_true(h3.size() == 16, "empty hash length 16");
    }

    // Test noop algorithm
    {
        auto algo = benchmark::make_noop_algorithm();
        test_support::expect_true(algo->name() == "noop", "noop name");
        test_support::expect_true(algo->version() == "1.0.0", "noop version");

        std::vector<uint8_t> data = {10, 20, 30, 40, 50};
        test_roundtrip(*algo, "noop small", data, "0");

        // Noop should not change size
        auto compressed = algo->compress(data, "0");
        test_support::expect_true(compressed.data.size() == data.size(), "noop no size change");
    }

    // Test RLE algorithm (toy, kept for reference)
    {
        auto algo = benchmark::make_rle_algorithm();
        test_support::expect_true(algo->name() == "rle", "rle name");

        std::vector<uint8_t> runs(1000, 0xAA);
        test_roundtrip(*algo, "rle uniform", runs, "1");
        auto compressed = algo->compress(runs, "1");
        test_support::expect_true(compressed.data.size() < runs.size(),
                                  "rle compresses uniform data");

        std::vector<uint8_t> varied;
        for (int i = 0; i < 256; ++i) varied.push_back(static_cast<uint8_t>(i));
        test_roundtrip(*algo, "rle varied", varied, "1");

        std::vector<uint8_t> empty;
        test_roundtrip(*algo, "rle empty", empty, "1");
    }

    // Test dictionary algorithm (toy, kept for reference)
    {
        auto algo = benchmark::make_dictionary_algorithm();
        test_support::expect_true(algo->name() == "dictionary", "dict name");

        std::vector<uint8_t> repeated;
        std::vector<uint8_t> pattern = {0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE};
        for (int i = 0; i < 100; ++i) {
            repeated.insert(repeated.end(), pattern.begin(), pattern.end());
        }
        test_roundtrip(*algo, "dict repeated", repeated, "3");

        std::vector<uint8_t> small = {1, 2, 3};
        test_roundtrip(*algo, "dict small", small, "3");

        std::vector<uint8_t> empty;
        test_roundtrip(*algo, "dict empty", empty, "3");
    }

    // Test deflate-lite algorithm (toy, kept for reference)
    {
        auto algo = benchmark::make_deflate_lite_algorithm();
        test_support::expect_true(algo->name() == "deflate_lite", "deflate_lite name");

        std::vector<uint8_t> pso_like;
        for (int block = 0; block < 20; ++block) {
            pso_like.push_back(0x01);
            pso_like.push_back(0x02);
            pso_like.push_back(0x03);
            pso_like.push_back(0x04);
            for (int i = 0; i < 50; ++i) pso_like.push_back(0x00);
        }
        test_roundtrip(*algo, "deflate_lite pso-like", pso_like, "6");

        std::vector<uint8_t> empty;
        test_roundtrip(*algo, "deflate_lite empty", empty, "6");
    }

    // ===== Real compression library tests =====
    // Generate a 256KB PSO payload for real algorithm testing
    auto pso_payload = benchmark::generate_pso_cache_payload(256 * 1024);
    test_support::expect_true(pso_payload.size() == 256 * 1024, "256KB payload generated");

    // Test zstd
    {
        auto algo = benchmark::make_zstd_algorithm();
        test_support::expect_true(algo->name() == "zstd", "zstd name");
        test_support::expect_true(!algo->version().empty(), "zstd version non-empty");

        test_roundtrip(*algo, "zstd-1", pso_payload, "1");
        test_roundtrip(*algo, "zstd-9", pso_payload, "9");

        auto c1 = algo->compress(pso_payload, "1");
        auto c9 = algo->compress(pso_payload, "9");
        test_support::expect_true(c1.data.size() < pso_payload.size(), "zstd-1 compresses");
        test_support::expect_true(c9.data.size() <= c1.data.size(), "zstd-9 <= zstd-1 size");
    }

    // Test lz4
    {
        auto algo = benchmark::make_lz4_algorithm();
        test_support::expect_true(algo->name() == "lz4", "lz4 name");
        test_support::expect_true(!algo->version().empty(), "lz4 version non-empty");

        test_roundtrip(*algo, "lz4-1", pso_payload, "1");
        test_roundtrip(*algo, "lz4-9", pso_payload, "9");

        auto c = algo->compress(pso_payload, "1");
        test_support::expect_true(c.data.size() < pso_payload.size(), "lz4 compresses");
    }

    // Test zlib
    {
        auto algo = benchmark::make_zlib_algorithm();
        test_support::expect_true(algo->name() == "zlib", "zlib name");
        test_support::expect_true(!algo->version().empty(), "zlib version non-empty");

        test_roundtrip(*algo, "zlib-1", pso_payload, "1");
        test_roundtrip(*algo, "zlib-6", pso_payload, "6");
        test_roundtrip(*algo, "zlib-9", pso_payload, "9");

        auto c1 = algo->compress(pso_payload, "1");
        auto c9 = algo->compress(pso_payload, "9");
        test_support::expect_true(c1.data.size() < pso_payload.size(), "zlib-1 compresses");
        test_support::expect_true(c9.data.size() <= c1.data.size(), "zlib-9 <= zlib-1 size");
    }

    // Test snappy
    {
        auto algo = benchmark::make_snappy_algorithm();
        test_support::expect_true(algo->name() == "snappy", "snappy name");
        test_support::expect_true(!algo->version().empty(), "snappy version non-empty");

        test_roundtrip(*algo, "snappy", pso_payload, "0");

        auto c = algo->compress(pso_payload, "0");
        test_support::expect_true(c.data.size() < pso_payload.size(), "snappy compresses");
    }

    // Test brotli
    {
        auto algo = benchmark::make_brotli_algorithm();
        test_support::expect_true(algo->name() == "brotli", "brotli name");
        test_support::expect_true(!algo->version().empty(), "brotli version non-empty");

        test_roundtrip(*algo, "brotli-1", pso_payload, "1");
        test_roundtrip(*algo, "brotli-6", pso_payload, "6");
        test_roundtrip(*algo, "brotli-11", pso_payload, "11");

        auto c1 = algo->compress(pso_payload, "1");
        auto c11 = algo->compress(pso_payload, "11");
        test_support::expect_true(c1.data.size() < pso_payload.size(), "brotli-1 compresses");
        test_support::expect_true(c11.data.size() <= c1.data.size(), "brotli-11 <= brotli-1 size");
    }

    // Test lzma
    {
        auto algo = benchmark::make_lzma_algorithm();
        test_support::expect_true(algo->name() == "lzma", "lzma name");
        test_support::expect_true(!algo->version().empty(), "lzma version non-empty");

        test_roundtrip(*algo, "lzma-1", pso_payload, "1");
        test_roundtrip(*algo, "lzma-6", pso_payload, "6");

        auto c1 = algo->compress(pso_payload, "1");
        auto c6 = algo->compress(pso_payload, "6");
        test_support::expect_true(c1.data.size() < pso_payload.size(), "lzma-1 compresses");
        test_support::expect_true(c6.data.size() <= c1.data.size(), "lzma-6 <= lzma-1 size");
    }

    // ===== Oodle compression tests =====
    // Test oodle_kraken
    {
        auto algo = benchmark::make_oodle_kraken_algorithm();
        test_support::expect_true(algo->name() == "oodle_kraken", "oodle_kraken name");
        test_support::expect_true(algo->version() == "2.9.8", "oodle_kraken version");

        test_roundtrip(*algo, "oodle_kraken-1", pso_payload, "1");
        test_roundtrip(*algo, "oodle_kraken-4", pso_payload, "4");

        auto c = algo->compress(pso_payload, "1");
        test_support::expect_true(c.data.size() < pso_payload.size(), "oodle_kraken compresses");
    }

    // Test oodle_leviathan
    {
        auto algo = benchmark::make_oodle_leviathan_algorithm();
        test_support::expect_true(algo->name() == "oodle_leviathan", "oodle_leviathan name");
        test_support::expect_true(algo->version() == "2.9.8", "oodle_leviathan version");

        test_roundtrip(*algo, "oodle_leviathan-1", pso_payload, "1");
        test_roundtrip(*algo, "oodle_leviathan-4", pso_payload, "4");

        auto c = algo->compress(pso_payload, "1");
        test_support::expect_true(c.data.size() < pso_payload.size(), "oodle_leviathan compresses");
    }

    // Test oodle_mermaid
    {
        auto algo = benchmark::make_oodle_mermaid_algorithm();
        test_support::expect_true(algo->name() == "oodle_mermaid", "oodle_mermaid name");
        test_support::expect_true(algo->version() == "2.9.8", "oodle_mermaid version");

        test_roundtrip(*algo, "oodle_mermaid-1", pso_payload, "1");
        test_roundtrip(*algo, "oodle_mermaid-4", pso_payload, "4");

        auto c = algo->compress(pso_payload, "1");
        test_support::expect_true(c.data.size() < pso_payload.size(), "oodle_mermaid compresses");
    }

    // Test oodle_selkie
    {
        auto algo = benchmark::make_oodle_selkie_algorithm();
        test_support::expect_true(algo->name() == "oodle_selkie", "oodle_selkie name");
        test_support::expect_true(algo->version() == "2.9.8", "oodle_selkie version");

        test_roundtrip(*algo, "oodle_selkie-1", pso_payload, "1");
        test_roundtrip(*algo, "oodle_selkie-4", pso_payload, "4");

        auto c = algo->compress(pso_payload, "1");
        test_support::expect_true(c.data.size() < pso_payload.size(), "oodle_selkie compresses");
    }

    // Test with real PSO payload generator data (toy algorithms)
    {
        auto payload = benchmark::generate_pso_cache_payload(4096);
        test_support::expect_true(payload.size() == 4096, "payload generator size");

        auto noop = benchmark::make_noop_algorithm();
        auto rle = benchmark::make_rle_algorithm();
        auto dict = benchmark::make_dictionary_algorithm();
        auto deflate = benchmark::make_deflate_lite_algorithm();

        test_roundtrip(*noop, "pso-noop", payload, "0");
        test_roundtrip(*rle, "pso-rle", payload, "1");
        test_roundtrip(*dict, "pso-dict", payload, "3");
        test_roundtrip(*deflate, "pso-deflate", payload, "6");
    }

    // Test payload with compressibility — verify with deterministic payloads on ALL real algorithms
    {
        auto high_compress = benchmark::generate_payload_with_compressibility(8192, 0.9);
        auto low_compress = benchmark::generate_payload_with_compressibility(8192, 0.1);
        test_support::expect_true(high_compress.size() == 8192, "high compressibility size");
        test_support::expect_true(low_compress.size() == 8192, "low compressibility size");

        // Test with all real algorithms (except noop which doesn't compress)
        struct AlgoFactory {
            std::string name;
            std::unique_ptr<benchmark::CompressionAlgorithm> (*make)();
        };
        // Use lambda wrappers since we can't take address of functions returning unique_ptr directly
        auto test_algo = [&](const std::string& name, benchmark::CompressionAlgorithm& algo) {
            auto c_high = algo.compress(high_compress, "1");
            auto c_low = algo.compress(low_compress, "1");
            test_support::expect_true(c_high.data.size() < c_low.data.size(),
                name + " high compressibility compresses better than low");
        };

        auto zstd = benchmark::make_zstd_algorithm();
        test_algo("zstd", *zstd);
        auto lz4 = benchmark::make_lz4_algorithm();
        test_algo("lz4", *lz4);
        auto zlib = benchmark::make_zlib_algorithm();
        test_algo("zlib", *zlib);
        auto snappy = benchmark::make_snappy_algorithm();
        test_algo("snappy", *snappy);
        auto brotli = benchmark::make_brotli_algorithm();
        test_algo("brotli", *brotli);
        auto lzma = benchmark::make_lzma_algorithm();
        test_algo("lzma", *lzma);
        auto kraken = benchmark::make_oodle_kraken_algorithm();
        test_algo("oodle_kraken", *kraken);
        auto leviathan = benchmark::make_oodle_leviathan_algorithm();
        test_algo("oodle_leviathan", *leviathan);
        auto mermaid = benchmark::make_oodle_mermaid_algorithm();
        test_algo("oodle_mermaid", *mermaid);
        auto selkie = benchmark::make_oodle_selkie_algorithm();
        test_algo("oodle_selkie", *selkie);

        // Also test toy algorithms
        auto rle = benchmark::make_rle_algorithm();
        test_algo("rle", *rle);
        auto dict = benchmark::make_dictionary_algorithm();
        test_algo("dictionary", *dict);
        auto deflate = benchmark::make_deflate_lite_algorithm();
        test_algo("deflate_lite", *deflate);
    }

    std::cout << "compression_algorithm_test: all checks completed\n";
    return test_support::finish();
}
