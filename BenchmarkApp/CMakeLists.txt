cmake_minimum_required(VERSION 3.20)
project(PSOBenchmark VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

set(BENCHMARK_ENABLE_OBJCXX OFF)
if(APPLE OR IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    enable_language(OBJCXX)
    set(BENCHMARK_ENABLE_OBJCXX ON)
endif()

enable_testing()

add_subdirectory(third_party)
add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(platform)
add_subdirectory(tools)
