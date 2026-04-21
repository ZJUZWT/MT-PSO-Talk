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
    compression/algorithms/oodle_algorithm.cpp
)

target_include_directories(benchmark_compression
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_compression
    PUBLIC
        benchmark_manifests
        benchmark_results
    PRIVATE
        tp_zstd
        tp_lz4
        tp_zlib
        tp_snappy
        tp_brotli
)

if(TARGET tp_oodle)
    target_link_libraries(benchmark_compression PRIVATE tp_oodle)
    target_compile_definitions(benchmark_compression PUBLIC BENCHMARK_HAS_OODLE=1)
else()
    target_compile_definitions(benchmark_compression PUBLIC BENCHMARK_HAS_OODLE=0)
endif()

# Real GPU backend sources (all compile on all platforms via #if guards)
set(VULKAN_BACKEND_SRC
    driver/vulkan/vulkan_context.cpp
    driver/vulkan/vulkan_pipeline.cpp
    driver/vulkan/vulkan_backend.cpp
)

set(OPENGL_BACKEND_SRC
    driver/opengl/egl_context.cpp
    driver/opengl/gl_program.cpp
    driver/opengl/opengl_backend.cpp
)

set(METAL_BACKEND_SRC
    driver/metal/metal_context.mm
    driver/metal/metal_pipeline.mm
    driver/metal/metal_backend.mm
)

if(BENCHMARK_ENABLE_OBJCXX)
    # Build the real Metal backend as Objective-C++ on Apple platforms.
    set_source_files_properties(
        driver/metal/metal_context.mm
        driver/metal/metal_pipeline.mm
        driver/metal/metal_backend.mm
        PROPERTIES LANGUAGE OBJCXX
    )
else()
    # Compile the stubbed non-Apple path as plain C++ so Android/Windows keep linking cleanly.
    set_source_files_properties(
        driver/metal/metal_context.mm
        driver/metal/metal_pipeline.mm
        driver/metal/metal_backend.mm
        PROPERTIES LANGUAGE CXX
    )
endif()

add_library(benchmark_graphics STATIC
    driver/common/apple_device_lookup.cpp
    driver/native/native_backend_stub.cpp
    driver/common/timing_model.cpp
    driver/common/backend_factory.cpp
    driver/common/device_info.cpp
    driver/mesa/mesa_opengl_backend.cpp
    driver/mesa/mesa_vulkan_backend.cpp
    driver/mesa/mesa_env_setup.cpp
    driver/native/native_opengl_backend.cpp
    driver/native/native_vulkan_backend.cpp
    driver/native/metal_backend.cpp
    driver/graphics_benchmark.cpp
    ${VULKAN_BACKEND_SRC}
    ${OPENGL_BACKEND_SRC}
    ${METAL_BACKEND_SRC}
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
