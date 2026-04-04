cmake_minimum_required(VERSION 3.20)
project(PSOBenchmark VERSION 0.1.0 LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(third_party)
add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(platform)
add_subdirectory(tools)
