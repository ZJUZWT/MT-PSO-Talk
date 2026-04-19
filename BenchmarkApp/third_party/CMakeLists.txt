# =============================================================================
# Vendored third-party compression libraries
# =============================================================================

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# --- lz4 (BSD-2-Clause) ---
add_library(tp_lz4 STATIC
    ${THIRD_PARTY_DIR}/lz4/lz4.c
    ${THIRD_PARTY_DIR}/lz4/lz4hc.c
)
target_include_directories(tp_lz4 PUBLIC ${THIRD_PARTY_DIR}/lz4)
target_compile_options(tp_lz4 PRIVATE -w)

# --- zstd (BSD-3-Clause) ---
file(GLOB ZSTD_COMMON_SRC ${THIRD_PARTY_DIR}/zstd/common/*.c)
file(GLOB ZSTD_COMPRESS_SRC ${THIRD_PARTY_DIR}/zstd/compress/*.c)
file(GLOB ZSTD_DECOMPRESS_SRC ${THIRD_PARTY_DIR}/zstd/decompress/*.c)
add_library(tp_zstd STATIC ${ZSTD_COMMON_SRC} ${ZSTD_COMPRESS_SRC} ${ZSTD_DECOMPRESS_SRC})
target_include_directories(tp_zstd
    PUBLIC ${THIRD_PARTY_DIR}/zstd
    PRIVATE ${THIRD_PARTY_DIR}/zstd/common
)
target_compile_definitions(tp_zstd PRIVATE ZSTD_MULTITHREAD=0 XXH_NAMESPACE=ZSTD_)
target_compile_options(tp_zstd PRIVATE -w)

# --- zlib (Zlib License) ---
file(GLOB ZLIB_SRC ${THIRD_PARTY_DIR}/zlib/*.c)
add_library(tp_zlib STATIC ${ZLIB_SRC})
target_include_directories(tp_zlib PUBLIC ${THIRD_PARTY_DIR}/zlib)
target_compile_definitions(tp_zlib PRIVATE _LARGEFILE64_SOURCE=1 HAVE_UNISTD_H=1)
target_compile_options(tp_zlib PRIVATE -w -Wno-error=implicit-function-declaration)

# --- snappy (BSD-3-Clause) ---
add_library(tp_snappy STATIC
    ${THIRD_PARTY_DIR}/snappy/snappy.cc
    ${THIRD_PARTY_DIR}/snappy/snappy-sinksource.cc
    ${THIRD_PARTY_DIR}/snappy/snappy-stubs-internal.cc
)
target_include_directories(tp_snappy PUBLIC ${THIRD_PARTY_DIR}/snappy)
target_compile_definitions(tp_snappy PRIVATE HAVE_CONFIG_H=1)
# Platform-specific SIMD flags for snappy
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    target_compile_definitions(tp_snappy PRIVATE SNAPPY_HAVE_NEON=1 SNAPPY_HAVE_SSSE3=0)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    target_compile_definitions(tp_snappy PRIVATE SNAPPY_HAVE_NEON=0 SNAPPY_HAVE_SSSE3=1)
else()
    target_compile_definitions(tp_snappy PRIVATE SNAPPY_HAVE_NEON=0 SNAPPY_HAVE_SSSE3=0)
endif()
target_compile_options(tp_snappy PRIVATE -w)

# --- brotli (MIT) ---
file(GLOB BROTLI_COMMON_SRC ${THIRD_PARTY_DIR}/brotli/common/*.c)
file(GLOB BROTLI_ENC_SRC ${THIRD_PARTY_DIR}/brotli/enc/*.c)
file(GLOB BROTLI_DEC_SRC ${THIRD_PARTY_DIR}/brotli/dec/*.c)
add_library(tp_brotli STATIC ${BROTLI_COMMON_SRC} ${BROTLI_ENC_SRC} ${BROTLI_DEC_SRC})
target_include_directories(tp_brotli PUBLIC ${THIRD_PARTY_DIR}/brotli/include)
# Brotli internal sources include relative paths like "../common/..."
target_include_directories(tp_brotli PRIVATE ${THIRD_PARTY_DIR}/brotli)
target_compile_options(tp_brotli PRIVATE -w)

# --- oodle (Proprietary, locally supplied from a licensed UE OodleDataCompression SDK) ---
set(BENCHMARK_OODLE_ROOT "" CACHE PATH
    "Path to a licensed Oodle 2.9.12 SDK root that contains include/ and lib/")

if(BENCHMARK_OODLE_ROOT)
    set(_benchmark_oodle_root_hint "${BENCHMARK_OODLE_ROOT}")
elseif(DEFINED ENV{BENCHMARK_OODLE_ROOT} AND NOT "$ENV{BENCHMARK_OODLE_ROOT}" STREQUAL "")
    set(_benchmark_oodle_root_hint "$ENV{BENCHMARK_OODLE_ROOT}")
else()
    set(_benchmark_oodle_root_hint "${THIRD_PARTY_DIR}/oodle/2.9.12")
endif()

file(TO_CMAKE_PATH "${_benchmark_oodle_root_hint}" OODLE_ROOT_DIR)
set(OODLE_INCLUDE_DIR "${OODLE_ROOT_DIR}/include")
set(OODLE_LIBRARY_PATH "")

if(ANDROID)
    set(OODLE_LIBRARY_PATH
        ${OODLE_ROOT_DIR}/lib/Android/${ANDROID_ABI}/liboo2coreandroid.a
    )
elseif(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(OODLE_IOS_VARIANT liboo2coreios.a)
    if(CMAKE_OSX_SYSROOT MATCHES "iphonesimulator")
        set(OODLE_IOS_VARIANT liboo2coreios.sim.a)
    endif()
    set(OODLE_LIBRARY_PATH
        ${OODLE_ROOT_DIR}/lib/IOS/${OODLE_IOS_VARIANT}
    )
elseif(APPLE)
    set(OODLE_LIBRARY_PATH
        ${OODLE_ROOT_DIR}/lib/Mac/liboo2coremac64.a
    )
elseif(WIN32)
    set(OODLE_WINDOWS_DIR Win64)
    set(OODLE_WINDOWS_LIB oo2core_win64.lib)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64"
       OR CMAKE_VS_PLATFORM_NAME MATCHES "ARM64")
        set(OODLE_WINDOWS_DIR WinArm64)
        set(OODLE_WINDOWS_LIB oo2core_winuwparm64.lib)
    endif()
    set(OODLE_LIBRARY_PATH
        ${OODLE_ROOT_DIR}/lib/${OODLE_WINDOWS_DIR}/${OODLE_WINDOWS_LIB}
    )
endif()

if(OODLE_LIBRARY_PATH
   AND EXISTS "${OODLE_LIBRARY_PATH}"
   AND EXISTS "${OODLE_INCLUDE_DIR}/oodle2.h"
   AND EXISTS "${OODLE_INCLUDE_DIR}/oodle2base.h")
    add_library(tp_oodle STATIC IMPORTED GLOBAL)
    set_target_properties(tp_oodle PROPERTIES
        IMPORTED_LOCATION "${OODLE_LIBRARY_PATH}"
        INTERFACE_INCLUDE_DIRECTORIES "${OODLE_INCLUDE_DIR}"
    )
    message(STATUS "BenchmarkApp: Oodle SDK enabled from ${OODLE_ROOT_DIR}")
else()
    message(STATUS
        "BenchmarkApp: Oodle SDK not found for this platform. "
        "Set BENCHMARK_OODLE_ROOT (or env BENCHMARK_OODLE_ROOT) to a licensed "
        "Oodle 2.9.12 SDK root to enable Oodle algorithms.")
endif()
