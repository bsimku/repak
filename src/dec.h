#ifndef DEC_H
#define DEC_H

#include <stdio.h>

#include "dec_deflate.h"
#include "dec_none.h"
#include "dec_zstd.h"

typedef struct {
    dec_deflate_t *deflate;
    dec_none_t *none;
    dec_zstd_t *zstd;
} dec_ctx_t;

dec_ctx_t *dec_init();
int dec_stream(dec_ctx_t *ctx, dec_read_callback read, void *opaque, int compress_type, void **data, size_t *size_dec);
void dec_free(dec_ctx_t *ctx);

#endif
