#ifndef DEC_ZSTD_H
#define DEC_ZSTD_H

#include <stddef.h>
#include <stdio.h>

#include <zstd.h>

#include "dec_common.h"

typedef struct {
    ZSTD_DCtx *ctx;
    ZSTD_inBuffer input;
    ZSTD_outBuffer output;
    void      *buffer_in;
    void      *buffer_out;
    size_t     buffer_in_size;
    size_t     buffer_out_size;
} dec_zstd_t;

dec_zstd_t *dec_zstd_init();
int dec_zstd_stream(dec_zstd_t *zstd, dec_read_callback read, void *opaque, void **data, size_t *size_dec);
void dec_zstd_free(dec_zstd_t *zstd);

#endif
