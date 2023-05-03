#include "comp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

static bool try_init_zstd(comp_ctx_t *ctx, comp_options_t *options) {
    if (ctx->zstd)
        return true;

    return (ctx->zstd = comp_zstd_init(options));
}

comp_ctx_t *comp_init() {
    comp_ctx_t *ctx = safe_alloc(sizeof(comp_ctx_t));

    ctx->zstd = NULL;

    return ctx;
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

size_t comp_stream(comp_ctx_t *ctx, comp_options_t *options, comp_write_callback write, void *opaque, void *data, size_t size) {
    switch (options->type) {
        case COMPRESS_TYPE_NONE: {
            if (!write(opaque, data, size))
                return COMP_ERROR;

            return size;
        }
        case COMPRESS_TYPE_DEFLATE:
            fprintf(stderr, "UNIMPLEMENTED\n");
            return COMP_ERROR;
        case COMPRESS_TYPE_ZSTD:
            if (!try_init_zstd(ctx, options))
                return COMP_ERROR;

            return comp_zstd_stream(ctx->zstd, write, opaque, data, size);
    }

    return COMP_ERROR;
}

void comp_free(comp_ctx_t *ctx) {
    free(ctx);
}

