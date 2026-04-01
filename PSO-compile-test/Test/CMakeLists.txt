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

add_executable(result_json_test
    Results/tests/result_json_test.cpp
)

target_link_libraries(result_json_test PRIVATE benchmark_results)

add_test(NAME result_json_test COMMAND result_json_test)
