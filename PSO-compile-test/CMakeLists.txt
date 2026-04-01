cmake_minimum_required(VERSION 3.15)
project(PSOCompileTester VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Mesa source path
set(MESA_SOURCE_DIR ``/libs/mesa)

# Source files
file(GLOB_RECURSE SOURCES ``/src/*.cpp)
file(GLOB_RECURSE HEADERS ``/src/*.h)

# Main executable
add_executable(pso_compile_tester `` ``)

target_include_directories(pso_compile_tester PRIVATE ``/src)

# Platform detection
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS)
else()
    add_definitions(-DPLATFORM_LINUX)
endif()

# Tests subdirectory
option(BUILD_TESTS "Build test suite" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
