# --- Find compression libraries ---
find_package(PkgConfig REQUIRED)

pkg_check_modules(ZSTD REQUIRED IMPORTED_TARGET libzstd)
pkg_check_modules(LZ4 REQUIRED IMPORTED_TARGET liblz4)
pkg_check_modules(BROTLI_ENC REQUIRED IMPORTED_TARGET libbrotlienc)
pkg_check_modules(BROTLI_DEC REQUIRED IMPORTED_TARGET libbrotlidec)
pkg_check_modules(LZMA REQUIRED IMPORTED_TARGET liblzma)

# snappy - no pkg-config, use find_path/find_library
find_path(SNAPPY_INCLUDE_DIR snappy.h HINTS /opt/homebrew/opt/snappy/include)
find_library(SNAPPY_LIBRARY snappy HINTS /opt/homebrew/opt/snappy/lib)

# zlib - system provided
find_package(ZLIB REQUIRED)

# Oodle Data Compression (from UE SDK)
set(OODLE_SDK_DIR "/Users/swannzhang/Workspace/CppProjects/UnrealEngine/Engine/Source/Runtime/OodleDataCompression/Sdks/2.9.8")
set(OODLE_INCLUDE_DIR "${OODLE_SDK_DIR}/include")
set(OODLE_LIBRARY "${OODLE_SDK_DIR}/lib/Mac/liboo2coremac64.a")

# ============================================================
# Library targets
# ============================================================

add_library(benchmark_results STATIC
    results/schema/result_types.cpp
    results/schema/result_json.cpp
)

target_include_directories(benchmark_results
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_manifests STATIC
    shaders/manifests/shader_workload.cpp
    compression/baseline_manifest.cpp
)

target_include_directories(benchmark_manifests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_compression STATIC
    compression/compression_job.cpp
    compression/decompression_replay.cpp
    compression/payload_generator.cpp
    compression/compression_benchmark.cpp
    compression/algorithms/hash_util.cpp
    compression/algorithms/rle_algorithm.cpp
    compression/algorithms/dictionary_algorithm.cpp
    compression/algorithms/deflate_lite_algorithm.cpp
    compression/algorithms/noop_algorithm.cpp
    compression/algorithms/zstd_algorithm.cpp
    compression/algorithms/lz4_algorithm.cpp
    compression/algorithms/zlib_algorithm.cpp
    compression/algorithms/snappy_algorithm.cpp
    compression/algorithms/brotli_algorithm.cpp
    compression/algorithms/lzma_algorithm.cpp
    compression/algorithms/oodle_algorithm.cpp
)

target_include_directories(benchmark_compression
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${SNAPPY_INCLUDE_DIR}
        ${OODLE_INCLUDE_DIR}
)

target_link_libraries(benchmark_compression
    PUBLIC
        benchmark_manifests
        benchmark_results
    PRIVATE
        PkgConfig::ZSTD
        PkgConfig::LZ4
        PkgConfig::BROTLI_ENC
        PkgConfig::BROTLI_DEC
        PkgConfig::LZMA
        ${SNAPPY_LIBRARY}
        ZLIB::ZLIB
        ${OODLE_LIBRARY}
)

add_library(benchmark_graphics STATIC
    driver/native/native_backend_stub.cpp
    driver/common/timing_model.cpp
    driver/common/backend_factory.cpp
    driver/mesa/mesa_opengl_backend.cpp
    driver/mesa/mesa_vulkan_backend.cpp
    driver/native/native_opengl_backend.cpp
    driver/native/native_vulkan_backend.cpp
    driver/native/metal_backend.cpp
    driver/graphics_benchmark.cpp
)

target_include_directories(benchmark_graphics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_graphics
    PUBLIC
        benchmark_results
        benchmark_manifests
)

add_library(benchmark_mesa STATIC
    driver/mesa/mesa_trace_parser.cpp
)

target_include_directories(benchmark_mesa
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_exporters STATIC
    results/exporters/json_exporter.cpp
    results/exporters/csv_exporter.cpp
)

target_include_directories(benchmark_exporters
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_exporters
    PUBLIC
        benchmark_results
)

add_library(benchmark_runner STATIC
    runner/benchmark_runner.cpp
)

target_include_directories(benchmark_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_orchestrator STATIC
    orchestrator/benchmark_orchestrator.cpp
)

target_include_directories(benchmark_orchestrator
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_orchestrator
    PUBLIC
        $<LINK_LIBRARY:WHOLE_ARCHIVE,benchmark_graphics>
        $<LINK_LIBRARY:WHOLE_ARCHIVE,benchmark_compression>
        benchmark_exporters
        benchmark_runner
        benchmark_manifests
)

add_executable(benchmark_main
    orchestrator/benchmark_main.cpp
)

target_link_libraries(benchmark_main
    PRIVATE
        benchmark_orchestrator
)
