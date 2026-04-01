add_executable(test_support_smoke
    TestSupport/test_main.cpp
)

target_include_directories(test_support_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME test_support_smoke COMMAND test_support_smoke)

add_library(benchmark_results STATIC
    Results/schema/result_types.cpp
    Results/schema/result_json.cpp
)

target_include_directories(benchmark_results
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_manifests STATIC
    Shaders/manifests/shader_workload.cpp
    Compression/baseline_manifest.cpp
)

target_include_directories(benchmark_manifests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_compression STATIC
    Compression/compression_job.cpp
    Compression/decompression_replay.cpp
)

target_include_directories(benchmark_compression
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_compression
    PUBLIC
        benchmark_manifests
        benchmark_results
)

add_library(benchmark_graphics STATIC
    Graphics/native/native_backend_stub.cpp
)

target_include_directories(benchmark_graphics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(benchmark_graphics
    PUBLIC
        benchmark_results
)

add_library(benchmark_mesa STATIC
    Mesa/adapters/mesa_trace_parser.cpp
)

target_include_directories(benchmark_mesa
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(benchmark_runner STATIC
    Runner/benchmark_runner.cpp
)

target_include_directories(benchmark_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(result_json_test
    Results/tests/result_json_test.cpp
)

target_link_libraries(result_json_test PRIVATE benchmark_results)

add_test(NAME result_json_test COMMAND result_json_test)

add_executable(shader_workload_test
    Shaders/tests/shader_workload_test.cpp
)

target_link_libraries(shader_workload_test PRIVATE benchmark_manifests)

add_test(NAME shader_workload_test COMMAND shader_workload_test)

add_executable(baseline_manifest_test
    Compression/tests/baseline_manifest_test.cpp
)

target_link_libraries(baseline_manifest_test PRIVATE benchmark_manifests)

add_test(NAME baseline_manifest_test COMMAND baseline_manifest_test)

add_executable(decompression_replay_test
    Compression/tests/decompression_replay_test.cpp
)

target_link_libraries(decompression_replay_test PRIVATE benchmark_compression)

add_test(NAME decompression_replay_test COMMAND decompression_replay_test)

add_executable(benchmark_runner_test
    Runner/tests/benchmark_runner_test.cpp
)

target_link_libraries(benchmark_runner_test PRIVATE benchmark_runner)

add_test(NAME benchmark_runner_test COMMAND benchmark_runner_test)

add_executable(graphics_backend_test
    Graphics/tests/graphics_backend_test.cpp
)

target_link_libraries(graphics_backend_test PRIVATE benchmark_graphics)

add_test(NAME graphics_backend_test COMMAND graphics_backend_test)

add_executable(mesa_trace_parser_test
    Mesa/tests/mesa_trace_parser_test.cpp
)

target_link_libraries(mesa_trace_parser_test PRIVATE benchmark_mesa)

add_test(NAME mesa_trace_parser_test COMMAND mesa_trace_parser_test)
