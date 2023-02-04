#include "comp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

comp_ctx_t *comp_init() {
    comp_ctx_t *ctx = malloc(sizeof(comp_ctx_t));

    if (!ctx)
        return NULL;

    ctx->zstd = NULL;

    return ctx;
}

size_t comp_stream(comp_ctx_t *ctx, comp_options_t *options, void *data, size_t size, FILE *out_file, int flags) {
    switch (options->type) {
        case COMPRESS_TYPE_NONE: {
            const int ret = fwrite(data, size, 1, out_file);

            if (ret < 0) {
                fprintf(stderr, "fwrite() failed: %s\n", strerror(ret));
                return 0;
            }

            return size;
        }
        case COMPRESS_TYPE_DEFLATE:
            fprintf(stderr, "UNIMPLEMENTED\n");
            return COMP_ERROR;
        case COMPRESS_TYPE_ZSTD:
            if (!ctx->zstd && !(ctx->zstd = comp_zstd_init(options)))
                return COMP_ERROR;

            return comp_zstd_stream(ctx->zstd, data, size, out_file, flags);
    }

    return COMP_ERROR;
}

void comp_free(comp_ctx_t *ctx) {
    free(ctx);
}

