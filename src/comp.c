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

bool try_init_zstd(comp_ctx_t *ctx, comp_options_t *options) {
    if (ctx->zstd)
        return true;

    return (ctx->zstd = comp_zstd_init(options));
}

bool comp_set_size(comp_ctx_t *ctx, comp_options_t *options, const size_t size) {
    switch (options->type) {
        default:
            break;
        case COMPRESS_TYPE_ZSTD:
            if (!try_init_zstd(ctx, options))
                return false;

            comp_zstd_set_size(ctx->zstd, size);
    }

    return true;
}

size_t comp_stream(comp_ctx_t *ctx, comp_options_t *options, void *data, size_t size, FILE *out_file) {
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
            if (!try_init_zstd(ctx, options))
                return COMP_ERROR;

            return comp_zstd_stream(ctx->zstd, data, size, out_file);
    }

    return COMP_ERROR;
}

void comp_free(comp_ctx_t *ctx) {
    free(ctx);
}

