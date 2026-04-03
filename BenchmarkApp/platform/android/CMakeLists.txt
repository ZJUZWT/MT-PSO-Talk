# Android-specific build configuration
# Use with: cmake -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a

if(ANDROID)
    # Find Vulkan
    find_library(VULKAN_LIB vulkan)

    # Find EGL and GLESv3
    find_library(EGL_LIB EGL)
    find_library(GLES_LIB GLESv3)
    find_library(LOG_LIB log)

    # Android benchmark CLI
    add_executable(pso_benchmark benchmark_cli.cpp)
    target_link_libraries(pso_benchmark
        benchmark_orchestrator
        ${VULKAN_LIB}
        ${EGL_LIB}
        ${GLES_LIB}
        ${LOG_LIB}
    )
endif()
