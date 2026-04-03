#include "compression/baseline_manifest.h"
#include "test_support/assert.h"

#include <set>
#include <string>

static bool is_hex_string(const std::string& s) {
    for (char c : s) {
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')))
            return false;
    }
    return !s.empty();
}

int main() {
    // zstd
    {
        const auto zstd = benchmark::make_default_zstd_baseline();
        test_support::expect_equal(zstd.algorithm, std::string("zstd"), "zstd algorithm");
        test_support::expect_equal(zstd.version, std::string("1.5.5"), "zstd version");
        test_support::expect_equal(zstd.level, std::string("3"), "zstd level");
        test_support::expect_true(!zstd.baseline_package_id.empty(), "zstd package id");
        test_support::expect_equal(zstd.input_size, static_cast<int64_t>(262144), "zstd input size");
        test_support::expect_greater_than(zstd.input_size, static_cast<int64_t>(0), "zstd input_size > 0");
        test_support::expect_true(!zstd.compressed_hash.empty(), "zstd compressed_hash non-empty");
        test_support::expect_true(is_hex_string(zstd.compressed_hash), "zstd compressed_hash is hex");
        test_support::expect_true(!zstd.decompressed_hash.empty(), "zstd decompressed_hash non-empty");
        test_support::expect_true(is_hex_string(zstd.decompressed_hash), "zstd decompressed_hash is hex");
    }

    // lz4
    {
        const auto lz4 = benchmark::make_lz4_baseline();
        test_support::expect_equal(lz4.algorithm, std::string("lz4"), "lz4 algorithm");
        test_support::expect_equal(lz4.version, std::string("1.9.4"), "lz4 version");
        test_support::expect_equal(lz4.level, std::string("1"), "lz4 level");
        test_support::expect_equal(lz4.input_size, static_cast<int64_t>(262144), "lz4 input size");
        test_support::expect_greater_than(lz4.input_size, static_cast<int64_t>(0), "lz4 input_size > 0");
        test_support::expect_true(!lz4.baseline_package_id.empty(), "lz4 package id");
        test_support::expect_true(!lz4.compressed_hash.empty(), "lz4 compressed_hash non-empty");
        test_support::expect_true(is_hex_string(lz4.compressed_hash), "lz4 compressed_hash is hex");
        test_support::expect_true(!lz4.decompressed_hash.empty(), "lz4 decompressed_hash non-empty");
        test_support::expect_true(is_hex_string(lz4.decompressed_hash), "lz4 decompressed_hash is hex");
    }

    // zlib
    {
        const auto zlib = benchmark::make_zlib_baseline();
        test_support::expect_equal(zlib.algorithm, std::string("zlib"), "zlib algorithm");
        test_support::expect_equal(zlib.version, std::string("1.3"), "zlib version");
        test_support::expect_equal(zlib.level, std::string("6"), "zlib level");
        test_support::expect_equal(zlib.input_size, static_cast<int64_t>(262144), "zlib input size");
        test_support::expect_greater_than(zlib.input_size, static_cast<int64_t>(0), "zlib input_size > 0");
        test_support::expect_true(!zlib.baseline_package_id.empty(), "zlib package id");
        test_support::expect_true(!zlib.compressed_hash.empty(), "zlib compressed_hash non-empty");
        test_support::expect_true(is_hex_string(zlib.compressed_hash), "zlib compressed_hash is hex");
    }

    // snappy
    {
        const auto snappy = benchmark::make_snappy_baseline();
        test_support::expect_equal(snappy.algorithm, std::string("snappy"), "snappy algorithm");
        test_support::expect_equal(snappy.version, std::string("1.1.10"), "snappy version");
        test_support::expect_equal(snappy.level, std::string("default"), "snappy level");
        test_support::expect_equal(snappy.input_size, static_cast<int64_t>(262144), "snappy input size");
        test_support::expect_greater_than(snappy.input_size, static_cast<int64_t>(0), "snappy input_size > 0");
        test_support::expect_true(!snappy.baseline_package_id.empty(), "snappy package id");
        test_support::expect_true(!snappy.compressed_hash.empty(), "snappy compressed_hash non-empty");
        test_support::expect_true(is_hex_string(snappy.compressed_hash), "snappy compressed_hash is hex");
    }

    // make_all_compression_baselines returns exactly 4 unique algorithms
    {
        const auto all = benchmark::make_all_compression_baselines();
        test_support::expect_equal(all.size(), static_cast<size_t>(4), "all baselines count");
        test_support::expect_equal(all[0].algorithm, std::string("zstd"), "all[0] algorithm");
        test_support::expect_equal(all[1].algorithm, std::string("lz4"), "all[1] algorithm");
        test_support::expect_equal(all[2].algorithm, std::string("zlib"), "all[2] algorithm");
        test_support::expect_equal(all[3].algorithm, std::string("snappy"), "all[3] algorithm");

        std::set<std::string> algos;
        for (const auto& b : all) {
            algos.insert(b.algorithm);
            test_support::expect_greater_than(b.input_size, static_cast<int64_t>(0),
                b.algorithm + " input_size > 0");
        }
        test_support::expect_equal(algos.size(), static_cast<size_t>(4), "4 unique algorithms");
    }

    return test_support::finish();
}
