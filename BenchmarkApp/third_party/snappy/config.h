// config.h - generated for vendored snappy build
#ifndef SNAPPY_CONFIG_H_
#define SNAPPY_CONFIG_H_

#define HAVE_CONFIG_H 1

// Platform-specific settings
#ifdef _WIN32
// Windows doesn't have these POSIX headers
#define HAVE_SYS_MMAN_H 0
#define HAVE_UNISTD_H 0
#define HAVE_SYS_UIO_H 0
#else
#define HAVE_SYS_MMAN_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_UIO_H 1
#endif

#endif  // SNAPPY_CONFIG_H_
