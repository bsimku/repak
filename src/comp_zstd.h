#ifndef COMP_ZSTD_H
#define COMP_ZSTD_H

#include <stddef.h>
#include <stdio.h>

#include <zstd.h>

#include "comp_common.h"

typedef struct {
    ZSTD_CCtx *ctx;
    ZSTD_inBuffer input;
    ZSTD_outBuffer output;
    void      *buffer_out;
    size_t     buffer_out_size;
} comp_zstd_t;

comp_zstd_t *comp_zstd_init(comp_options_t *options);
size_t comp_zstd_stream(comp_zstd_t *zstd, void *data, size_t size, FILE *out_file, int flags);
void comp_zstd_free(comp_zstd_t *zstd);

#endif
