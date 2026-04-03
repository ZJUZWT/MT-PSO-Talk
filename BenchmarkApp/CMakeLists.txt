cmake_minimum_required(VERSION 3.20)
project(PSOBenchmark VERSION 0.1.0 LANGUAGES C CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Homebrew prefix paths for compression libraries
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/zstd")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/lz4")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/brotli")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/xz")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/snappy")

# PKG_CONFIG_PATH for pkg-config based discovery
set(ENV{PKG_CONFIG_PATH} "/opt/homebrew/opt/zstd/lib/pkgconfig:/opt/homebrew/opt/lz4/lib/pkgconfig:/opt/homebrew/opt/brotli/lib/pkgconfig:/opt/homebrew/opt/xz/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

enable_testing()

add_subdirectory(lib)
add_subdirectory(tests)
add_subdirectory(platform)
