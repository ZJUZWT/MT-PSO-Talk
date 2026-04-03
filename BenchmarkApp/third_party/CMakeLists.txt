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

# --- oodle (Proprietary, precompiled from UE SDK) ---
add_library(tp_oodle STATIC IMPORTED GLOBAL)
set_target_properties(tp_oodle PROPERTIES
    IMPORTED_LOCATION ${THIRD_PARTY_DIR}/oodle/lib/Mac/liboo2coremac64.a
    INTERFACE_INCLUDE_DIRECTORIES ${THIRD_PARTY_DIR}/oodle/include
)
