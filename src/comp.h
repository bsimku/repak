#ifndef COMP_H
#define COMP_H

#include <stddef.h>
#include <stdio.h>

#include "comp_common.h"
#include "comp_zstd.h"

typedef struct {
    comp_zstd_t *zstd;
} comp_ctx_t;

comp_ctx_t *comp_init();
size_t comp_stream(comp_ctx_t *ctx, comp_options_t *options, void *data, size_t size, FILE *out_file, int flags);
void comp_free(comp_ctx_t *ctx);

#endif
