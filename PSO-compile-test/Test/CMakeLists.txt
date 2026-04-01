add_executable(test_support_smoke
    TestSupport/test_main.cpp
)

target_include_directories(test_support_smoke
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME test_support_smoke COMMAND test_support_smoke)
