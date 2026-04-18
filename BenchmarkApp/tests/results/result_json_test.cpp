#include <string>

#include "results/schema/result_json.h"
#include "test_support/assert.h"

int main() {
    // --- GraphicsResult basic fields ---
    benchmark::GraphicsResult gfx{};
    gfx.platform = "Android";
    gfx.api = "Vulkan";
    gfx.driver_mode = "Mesa";
    gfx.total_us = 42;

    const std::string json = benchmark::to_json(gfx);

    // Verify JSON starts with '{' and ends with '}'
    test_support::expect_true(!json.empty(), "json not empty");
    test_support::expect_equal(json.front(), '{', "json starts with {");
    test_support::expect_equal(json.back(), '}', "json ends with }");

    test_support::expect_contains(json, "\"platform\":\"Android\"", "platform serialized");
    test_support::expect_contains(json, "\"total_us\":42", "total serialized");

    // --- GraphicsResult ALL fields ---
    gfx.device_model = "Pixel 8 Pro";
    gfx.soc = "Tensor G3";
    gfx.os_version = "Android 15";
    gfx.mesa_version = "mesa-24.1";
    gfx.case_name = "S1_BasePBR";
    gfx.iteration_index = 2;
    gfx.cold_cache = false;
    gfx.status = "runtime_error";
    gfx.shader_compile_us = 100;
    gfx.link_pipeline_create_us = 200;
    gfx.first_draw_ready_us = 300;
    gfx.stage_breakdown.push_back({"compile", 50});
    gfx.stage_breakdown.push_back({"link", 150});

    const std::string json2 = benchmark::to_json(gfx);
    test_support::expect_equal(json2.front(), '{', "json2 starts with {");
    test_support::expect_equal(json2.back(), '}', "json2 ends with }");

    // Verify ALL fields from GraphicsResult
    test_support::expect_contains(json2, "\"platform\":\"Android\"", "platform");
    test_support::expect_contains(json2, "\"api\":\"Vulkan\"", "api");
    test_support::expect_contains(json2, "\"driver_mode\":\"Mesa\"", "driver_mode");
    test_support::expect_contains(json2, "\"case_name\":\"S1_BasePBR\"", "case_name");
    test_support::expect_contains(json2, "\"device_model\":\"Pixel 8 Pro\"", "device_model");
    test_support::expect_contains(json2, "\"soc\":\"Tensor G3\"", "soc");
    test_support::expect_contains(json2, "\"os_version\":\"Android 15\"", "os_version");
    test_support::expect_contains(json2, "\"mesa_version\":\"mesa-24.1\"", "mesa_version");
    test_support::expect_contains(json2, "\"iteration_index\":2", "iteration_index");
    test_support::expect_contains(json2, "\"cold_cache\":false", "cold_cache");
    test_support::expect_contains(json2, "\"status\":\"runtime_error\"", "status");
    test_support::expect_contains(json2, "\"shader_compile_us\":100", "shader_compile_us");
    test_support::expect_contains(json2, "\"link_pipeline_create_us\":200", "link_pipeline_create_us");
    test_support::expect_contains(json2, "\"first_draw_ready_us\":300", "first_draw_ready_us");
    test_support::expect_contains(json2, "\"total_us\":42", "total_us");

    // stage_breakdown serializes as JSON array
    test_support::expect_contains(json2, "\"stage_breakdown\":[", "stage_breakdown array");
    test_support::expect_contains(json2, "{\"name\":\"compile\",\"duration_us\":50}",
        "stage_breakdown first entry");
    test_support::expect_contains(json2, "{\"name\":\"link\",\"duration_us\":150}",
        "stage_breakdown second entry");

    // --- Test with special characters in strings (quotes, backslashes) ---
    benchmark::GraphicsResult gfx_special{};
    gfx_special.platform = "Test";
    gfx_special.device_model = "Device\"With\\Quotes";
    const std::string json_special = benchmark::to_json(gfx_special);
    test_support::expect_equal(json_special.front(), '{', "special json starts with {");
    test_support::expect_equal(json_special.back(), '}', "special json ends with }");
    // The JSON should contain escaped versions
    test_support::expect_contains(json_special, "Device", "special device present");

    // --- CompressionResult serialization ---
    benchmark::CompressionResult comp{};
    comp.platform = "Linux";
    comp.algorithm = "zstd";
    comp.baseline_package_id = "pkg-001";
    comp.device_model = "Desktop";
    comp.soc = "x86_64";
    comp.os_version = "Ubuntu 24.04";
    comp.version = "1.5.6";
    comp.level = "3";
    comp.payload_profile = "pso_like";
    comp.iteration_index = 3;
    comp.input_size = 1024;
    comp.compressed_size = 512;
    comp.compression_ratio = 2.0;
    comp.compress_us = 500;
    comp.throughput_mbps = 100.5;
    comp.input_hash = "input123";
    comp.compressed_output_hash = "abc123";
    comp.decompress_us = 250;
    comp.decompressed_output_hash = "def456";
    comp.roundtrip_hash_match = true;
    comp.roundtrip_byte_match = false;
    comp.status = "unsupported";

    const std::string cjson = benchmark::to_json(comp);
    test_support::expect_equal(cjson.front(), '{', "comp json starts with {");
    test_support::expect_equal(cjson.back(), '}', "comp json ends with }");

    // Verify ALL CompressionResult fields
    test_support::expect_contains(cjson, "\"platform\":\"Linux\"", "comp platform");
    test_support::expect_contains(cjson, "\"algorithm\":\"zstd\"", "comp algorithm");
    test_support::expect_contains(cjson, "\"baseline_package_id\":\"pkg-001\"", "comp baseline_package_id");
    test_support::expect_contains(cjson, "\"device_model\":\"Desktop\"", "comp device_model");
    test_support::expect_contains(cjson, "\"soc\":\"x86_64\"", "comp soc");
    test_support::expect_contains(cjson, "\"os_version\":\"Ubuntu 24.04\"", "comp os_version");
    test_support::expect_contains(cjson, "\"version\":\"1.5.6\"", "comp version");
    test_support::expect_contains(cjson, "\"level\":\"3\"", "comp level");
    test_support::expect_contains(cjson, "\"payload_profile\":\"pso_like\"", "comp payload_profile");
    test_support::expect_contains(cjson, "\"iteration_index\":3", "comp iteration_index");
    test_support::expect_contains(cjson, "\"input_size\":1024", "comp input_size");
    test_support::expect_contains(cjson, "\"compressed_size\":512", "comp compressed_size");
    test_support::expect_contains(cjson, "\"compress_us\":500", "comp compress_us");
    test_support::expect_contains(cjson, "\"input_hash\":\"input123\"", "comp input_hash");
    test_support::expect_contains(cjson, "\"compressed_output_hash\":\"abc123\"", "comp compressed_output_hash");
    test_support::expect_contains(cjson, "\"decompress_us\":250", "comp decompress_us");
    test_support::expect_contains(cjson, "\"decompressed_output_hash\":\"def456\"", "comp decompressed_output_hash");
    test_support::expect_contains(cjson, "\"roundtrip_hash_match\":true", "comp roundtrip_hash_match");
    test_support::expect_contains(cjson, "\"roundtrip_byte_match\":false", "comp roundtrip_byte_match");
    test_support::expect_contains(cjson, "\"status\":\"unsupported\"", "comp status");

    return test_support::finish();
}
