// snappy-stubs-public.h - generated for vendored build
#ifndef THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_
#define THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_

#include <cstddef>
#include <cstdint>
#include <string>

#if defined(_WIN32)
// Windows doesn't have sys/uio.h, define iovec manually
struct iovec {
  void* iov_base;
  size_t iov_len;
};
#elif defined(__has_include)
#if __has_include(<sys/uio.h>)
#include <sys/uio.h>
#endif
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/uio.h>
#endif

#define SNAPPY_MAJOR 1
#define SNAPPY_MINOR 2
#define SNAPPY_PATCHLEVEL 1
#define SNAPPY_VERSION ((SNAPPY_MAJOR << 16) | (SNAPPY_MINOR << 8) | SNAPPY_PATCHLEVEL)

namespace snappy {
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;
}  // namespace snappy

#endif  // THIRD_PARTY_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_
