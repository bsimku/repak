#ifndef COMP_ZSTD_H
#define COMP_ZSTD_H

#include <stdbool.h>
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
    size_t     data_size;
    size_t     total_data_size;
} comp_zstd_t;

comp_zstd_t *comp_zstd_init(comp_options_t *options);
bool comp_zstd_set_size(comp_zstd_t *zstd, const size_t size);
size_t comp_zstd_stream(comp_zstd_t *zstd, void *data, size_t size, FILE *out_file);
void comp_zstd_free(comp_zstd_t *zstd);

#endif
