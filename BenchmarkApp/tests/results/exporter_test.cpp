#include <algorithm>
#include <string>
#include <vector>

#include "results/exporters/csv_exporter.h"
#include "results/exporters/json_exporter.h"
#include "test_support/assert.h"

static int count_char(const std::string& s, char c) {
    return static_cast<int>(std::count(s.begin(), s.end(), c));
}

int main() {
    // --- Single JSON export ---
    benchmark::GraphicsResult result{};
    result.platform = "Android";
    result.api = "OpenGL";

    const auto json = benchmark::export_graphics_json(result);
    test_support::expect_contains(json, "OpenGL", "json export");
    test_support::expect_equal(json.front(), '{', "single json starts with {");
    test_support::expect_equal(json.back(), '}', "single json ends with }");

    // --- CSV header functions + column count matching ---
    const auto gh = benchmark::graphics_csv_header();
    test_support::expect_contains(gh, "platform", "graphics csv header has platform");
    test_support::expect_contains(gh, "shader_compile_us", "graphics csv header has shader_compile_us");
    test_support::expect_contains(gh, "cold_cache", "graphics csv header has cold_cache");

    const auto ch = benchmark::compression_csv_header();
    test_support::expect_contains(ch, "algorithm", "compression csv header has algorithm");
    test_support::expect_contains(ch, "payload_profile", "compression csv header has payload_profile");
    test_support::expect_contains(ch, "iteration_index", "compression csv header has iteration_index");
    test_support::expect_contains(ch, "input_hash", "compression csv header has input_hash");
    test_support::expect_contains(ch, "throughput_mbps", "compression csv header has throughput_mbps");
    test_support::expect_contains(ch, "roundtrip_byte_match", "compression csv header has roundtrip_byte_match");
    test_support::expect_contains(ch, "status", "compression csv header has status");

    // --- CSV row with all fields + column count verification ---
    benchmark::GraphicsResult gr{};
    gr.platform = "Android";
    gr.api = "Vulkan";
    gr.driver_mode = "Mesa";
    gr.case_name = "tri";
    gr.device_model = "Pixel";
    gr.soc = "T3";
    gr.os_version = "Android15";
    gr.mesa_version = "mesa-24";
    gr.iteration_index = 1;
    gr.cold_cache = true;
    gr.status = "passed";
    gr.total_us = 99;
    gr.shader_compile_us = 50;
    gr.link_pipeline_create_us = 30;
    gr.first_draw_ready_us = 19;

    const auto row = benchmark::export_graphics_csv_row(gr);
    test_support::expect_contains(row, "Pixel", "csv row has device_model");
    test_support::expect_contains(row, "T3", "csv row has soc");
    test_support::expect_contains(row, "passed", "csv row has status");
    test_support::expect_contains(row, "Vulkan", "csv row has api");
    test_support::expect_contains(row, "Mesa", "csv row has driver_mode");
    test_support::expect_contains(row, "99", "csv row has total_us");

    // Verify CSV header and row have same column count (commas)
    int header_commas = count_char(gh, ',');
    int row_commas = count_char(row, ',');
    test_support::expect_equal(header_commas, row_commas,
        "CSV header comma count matches row comma count");

    // --- Compression CSV row column count check ---
    benchmark::CompressionResult cr{};
    cr.platform = "Linux";
    cr.algorithm = "zstd";
    cr.payload_profile = "pso_like";
    cr.iteration_index = 2;
    cr.input_hash = "input123";
    cr.compressed_output_hash = "abc123";
    cr.decompressed_output_hash = "def456";
    cr.roundtrip_hash_match = true;
    cr.roundtrip_byte_match = true;
    cr.status = "passed";
    cr.input_size = 1024;
    const auto crow = benchmark::export_compression_csv_row(cr);
    test_support::expect_contains(crow, "pso_like", "compression row has payload_profile");
    test_support::expect_contains(crow, "input123", "compression row has input hash");
    int ch_commas = count_char(ch, ',');
    int crow_commas = count_char(crow, ',');
    test_support::expect_equal(ch_commas, crow_commas,
        "Compression CSV header comma count matches row comma count");

    // --- Batch JSON array export ---
    std::vector<benchmark::GraphicsResult> gvec;
    benchmark::GraphicsResult g1{};
    g1.platform = "A";
    benchmark::GraphicsResult g2{};
    g2.platform = "B";
    gvec.push_back(g1);
    gvec.push_back(g2);

    const auto garr = benchmark::export_graphics_json_array(gvec);
    test_support::expect_equal(garr.front(), '[', "graphics array starts with [");
    test_support::expect_equal(garr.back(), ']', "graphics array ends with ]");
    test_support::expect_contains(garr, "\"platform\":\"A\"", "array has first");
    test_support::expect_contains(garr, "\"platform\":\"B\"", "array has second");

    std::vector<benchmark::CompressionResult> cvec;
    benchmark::CompressionResult c1{};
    c1.algorithm = "zstd";
    benchmark::CompressionResult c2{};
    c2.algorithm = "lz4";
    cvec.push_back(c1);
    cvec.push_back(c2);

    const auto carr = benchmark::export_compression_json_array(cvec);
    test_support::expect_equal(carr.front(), '[', "compression array starts with [");
    test_support::expect_equal(carr.back(), ']', "compression array ends with ]");
    test_support::expect_contains(carr, "\"zstd\"", "array has zstd");
    test_support::expect_contains(carr, "\"lz4\"", "array has lz4");

    // --- Empty array returns "[]" ---
    const auto empty_g = benchmark::export_graphics_json_array({});
    test_support::expect_equal(empty_g, std::string("[]"), "empty graphics array is []");

    const auto empty_c = benchmark::export_compression_json_array({});
    test_support::expect_equal(empty_c, std::string("[]"), "empty compression array is []");

    // --- Test with special characters (commas in device_model) ---
    benchmark::GraphicsResult gr_special{};
    gr_special.platform = "Android";
    gr_special.device_model = "Pixel,8,Pro";
    gr_special.soc = "Apple \"M1\" Pro";
    const auto special_json = benchmark::export_graphics_json(gr_special);
    test_support::expect_contains(special_json, "Pixel", "special json has device");

    const auto special_graphics_csv = benchmark::export_graphics_csv_row(gr_special);
    test_support::expect_contains(special_graphics_csv, "\"Pixel,8,Pro\"",
                                  "graphics csv quotes device_model with commas");
    test_support::expect_contains(special_graphics_csv, "\"Apple \"\"M1\"\" Pro\"",
                                  "graphics csv escapes embedded quotes");

    benchmark::CompressionResult cr_special{};
    cr_special.platform = "macOS";
    cr_special.algorithm = "zstd";
    cr_special.baseline_package_id = "compression-bench-zstd";
    cr_special.device_model = "MacBookPro18,1";
    cr_special.soc = "Apple M1 Pro";
    cr_special.os_version = "macOS 14.6.1";
    cr_special.version = "1.5.6";
    cr_special.level = "3";
    cr_special.payload_profile = "pso_like";
    cr_special.iteration_index = 0;
    cr_special.input_size = 1024;
    cr_special.compressed_size = 128;
    cr_special.compression_ratio = 8.0;
    cr_special.compress_us = 10;
    cr_special.throughput_mbps = 100.0;
    cr_special.input_hash = "input";
    cr_special.compressed_output_hash = "compressed";
    cr_special.decompress_us = 5;
    cr_special.decompressed_output_hash = "roundtrip";
    cr_special.roundtrip_hash_match = true;
    cr_special.roundtrip_byte_match = true;
    cr_special.status = "passed";

    const auto special_compression_csv = benchmark::export_compression_csv_row(cr_special);
    test_support::expect_contains(special_compression_csv, "\"MacBookPro18,1\"",
                                  "compression csv quotes device_model with commas");

    return test_support::finish();
}
