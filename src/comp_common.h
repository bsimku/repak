#ifndef COMP_COMMON_H
#define COMP_COMMON_H

#include <stdbool.h>
#include <stddef.h>

#define COMP_ERROR ((size_t) - 1)

typedef enum {
    COMPRESS_TYPE_NONE,
    COMPRESS_TYPE_DEFLATE,
    COMPRESS_TYPE_ZSTD
} compress_type_e;

typedef struct {
    compress_type_e type;
    int level;
    int threads;
} comp_options_t;

typedef bool (*comp_write_callback)(void *, void *, size_t);

#endif
