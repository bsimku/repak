#include "dec.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pak_flags.h"
#include "utils.h"

dec_ctx_t *dec_init() {
    dec_ctx_t *ctx = safe_alloc(sizeof(dec_ctx_t));

    ctx->deflate = NULL;
    ctx->zstd = NULL;
    ctx->none = NULL;

    return ctx;
}

int dec_stream(dec_ctx_t *ctx, dec_read_callback read, void *opaque, int file_flags, void **data, size_t *size_dec) {
    if (file_flags & PAK_FILE_FLAG_DEFLATE) {
        if (!ctx->deflate && !(ctx->deflate = dec_deflate_init()))
            return DEC_ERROR;

        return dec_deflate_stream(ctx->deflate, read, opaque, data, size_dec);
    }
    else if (file_flags & PAK_FILE_FLAG_ZSTD) {
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

