#include "dec.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PAK_COMPRESSION_NONE    0
#define PAK_COMPRESSION_DEFLATE 1
#define PAK_COMPRESSION_ZSTD    2

dec_ctx_t *dec_init() {
    dec_ctx_t *ctx = malloc(sizeof(dec_ctx_t));

    if (!ctx)
        return NULL;

    ctx->deflate = NULL;
    ctx->zstd = NULL;

    return ctx;
}

int dec_stream(dec_ctx_t *ctx, dec_read_callback read, void *opaque, int compress_type, void **data, size_t *size_dec) {
    if (compress_type & PAK_COMPRESSION_DEFLATE) {
        if (!ctx->deflate && !(ctx->deflate = dec_deflate_init()))
            return DEC_ERROR;

        return dec_deflate_stream(ctx->deflate, read, opaque, data, size_dec);
    }
    else if (compress_type & PAK_COMPRESSION_ZSTD) {
        if (!ctx->zstd && !(ctx->zstd = dec_zstd_init()))
            return DEC_ERROR;

        return dec_zstd_stream(ctx->zstd, read, opaque, data, size_dec);
    }

    if (!ctx->none && !(ctx->none = dec_none_init()))
        return DEC_ERROR;

    return dec_none_stream(ctx->none, read, opaque, data, size_dec);
}

void dec_free(dec_ctx_t *ctx) {
    if (ctx->deflate) {
        dec_deflate_free(ctx->deflate);
    }

    if (ctx->none) {
        dec_none_free(ctx->none);
    }

    if (ctx->zstd) {
        dec_zstd_free(ctx->zstd);
    }

    free(ctx);
}

