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

add_executable(benchmark_runner_test
    Runner/tests/benchmark_runner_test.cpp
)

target_link_libraries(benchmark_runner_test PRIVATE benchmark_runner)

add_test(NAME benchmark_runner_test COMMAND benchmark_runner_test)
