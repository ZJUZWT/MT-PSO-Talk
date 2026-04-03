# All test executables
# Include dirs: lib/ for library headers, tests/ for test_support

set(LIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lib)
set(TEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/tests)

# --- test_support_smoke ---
add_executable(test_support_smoke
    test_support/test_main.cpp
)

target_include_directories(test_support_smoke PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})

add_test(NAME test_support_smoke COMMAND test_support_smoke)

# --- result_json_test ---
add_executable(result_json_test
    results/result_json_test.cpp
)

target_include_directories(result_json_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(result_json_test PRIVATE benchmark_results)

add_test(NAME result_json_test COMMAND result_json_test)

# --- shader_workload_test ---
add_executable(shader_workload_test
    shaders/shader_workload_test.cpp
)

target_include_directories(shader_workload_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(shader_workload_test PRIVATE benchmark_manifests)

add_test(NAME shader_workload_test COMMAND shader_workload_test)

# --- baseline_manifest_test ---
add_executable(baseline_manifest_test
    compression/baseline_manifest_test.cpp
)

target_include_directories(baseline_manifest_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(baseline_manifest_test PRIVATE benchmark_manifests)

add_test(NAME baseline_manifest_test COMMAND baseline_manifest_test)

# --- decompression_replay_test ---
add_executable(decompression_replay_test
    compression/decompression_replay_test.cpp
)

target_include_directories(decompression_replay_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(decompression_replay_test PRIVATE benchmark_compression)

add_test(NAME decompression_replay_test COMMAND decompression_replay_test)

# --- benchmark_runner_test ---
add_executable(benchmark_runner_test
    runner/benchmark_runner_test.cpp
)

target_include_directories(benchmark_runner_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(benchmark_runner_test PRIVATE benchmark_runner)

add_test(NAME benchmark_runner_test COMMAND benchmark_runner_test)

# --- graphics_backend_test ---
add_executable(graphics_backend_test
    driver/graphics_backend_test.cpp
)

target_include_directories(graphics_backend_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(graphics_backend_test PRIVATE benchmark_graphics)

add_test(NAME graphics_backend_test COMMAND graphics_backend_test)

# --- timing_model_test ---
add_executable(timing_model_test
    driver/timing_model_test.cpp
)

target_include_directories(timing_model_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(timing_model_test PRIVATE benchmark_graphics)

add_test(NAME timing_model_test COMMAND timing_model_test)

# --- graphics_benchmark_test ---
add_executable(graphics_benchmark_test
    driver/graphics_benchmark_test.cpp
)

target_include_directories(graphics_benchmark_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(graphics_benchmark_test PRIVATE benchmark_graphics)

add_test(NAME graphics_benchmark_test COMMAND graphics_benchmark_test)

# --- mesa_trace_parser_test ---
add_executable(mesa_trace_parser_test
    driver/mesa_trace_parser_test.cpp
)

target_include_directories(mesa_trace_parser_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(mesa_trace_parser_test PRIVATE benchmark_mesa)

add_test(NAME mesa_trace_parser_test COMMAND mesa_trace_parser_test)

# --- compression_algorithm_test ---
add_executable(compression_algorithm_test
    compression/compression_algorithm_test.cpp
)

target_include_directories(compression_algorithm_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(compression_algorithm_test PRIVATE benchmark_compression)

add_test(NAME compression_algorithm_test COMMAND compression_algorithm_test)

# --- compression_benchmark_test ---
add_executable(compression_benchmark_test
    compression/compression_benchmark_test.cpp
)

target_include_directories(compression_benchmark_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(compression_benchmark_test PRIVATE $<LINK_LIBRARY:WHOLE_ARCHIVE,benchmark_compression>)

add_test(NAME compression_benchmark_test COMMAND compression_benchmark_test)

# --- exporter_test ---
add_executable(exporter_test
    results/exporter_test.cpp
)

target_include_directories(exporter_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(exporter_test PRIVATE benchmark_exporters)

add_test(NAME exporter_test COMMAND exporter_test)

# --- orchestrator_test ---
add_executable(orchestrator_test
    orchestrator/orchestrator_test.cpp
)

target_include_directories(orchestrator_test PRIVATE ${LIB_INCLUDE_DIR} ${TEST_INCLUDE_DIR})
target_link_libraries(orchestrator_test PRIVATE benchmark_orchestrator)

add_test(NAME orchestrator_test COMMAND orchestrator_test)
