cmake_minimum_required(VERSION 3.10.2)
project(ShaderCompileTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find Android libraries ──────────────────────────────────────────
find_library(LOG_LIB log)
find_library(ANDROID_LIB android)
find_library(GLES3_LIB GLESv3)
find_library(EGL_LIB EGL)

# ── Vulkan (available in NDK) ────────────────────────────────────────
find_library(VULKAN_LIB vulkan)

# ── Build the shared library ────────────────────────────────────────
add_library(shader_compile_test SHARED
    benchmark_main.cpp
    vulkan_test_android.cpp
    gles_test_android.cpp
    utils_android.cpp
)

target_link_libraries(shader_compile_test PRIVATE
    ${LOG_LIB}
    ${ANDROID_LIB}
    ${GLES3_LIB}
    ${EGL_LIB}
    ${VULKAN_LIB}
)
