# Platform-specific builds
if(ANDROID)
    add_subdirectory(android)
elseif(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    add_subdirectory(ios)
endif()

# Shell libraries for desktop builds (testing and development)
add_library(benchmark_android_shell STATIC
    android/native_entry.cpp
)

target_include_directories(benchmark_android_shell
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
)

add_library(benchmark_ios_shell STATIC
    ios/native_entry.mm
)

set_source_files_properties(
    ios/native_entry.mm
    PROPERTIES
        LANGUAGE CXX
)

target_include_directories(benchmark_ios_shell
    PUBLIC
        ${CMAKE_SOURCE_DIR}/lib
)

# Platform tests
add_executable(android_mode_bridge_test
    android/tests/android_mode_bridge_test.cpp
)

target_include_directories(android_mode_bridge_test PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(android_mode_bridge_test
    PRIVATE
        benchmark_android_shell
        benchmark_runner
)

add_test(NAME android_mode_bridge_test COMMAND android_mode_bridge_test)

add_executable(ios_mode_bridge_test
    ios/tests/ios_mode_bridge_test.cpp
)

target_include_directories(ios_mode_bridge_test PRIVATE ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(ios_mode_bridge_test
    PRIVATE
        benchmark_ios_shell
        benchmark_runner
)

add_test(NAME ios_mode_bridge_test COMMAND ios_mode_bridge_test)
