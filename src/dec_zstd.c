#include "dec_zstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zstd.h>

#include "utils.h"

dec_zstd_t *dec_zstd_init() {
    dec_zstd_t *zstd = safe_alloc(sizeof(dec_zstd_t));

    if (!(zstd->ctx = ZSTD_createDCtx()))
        goto error;

    zstd->buffer_in = NULL;
    zstd->buffer_out = NULL;

    zstd->buffer_in_size = ZSTD_DStreamInSize();
    zstd->buffer_out_size = ZSTD_DStreamOutSize();

    zstd->buffer_in = safe_alloc(zstd->buffer_in_size);
    zstd->buffer_out = safe_alloc(zstd->buffer_out_size);

    zstd->input.src = zstd->buffer_in;
    zstd->input.size = 0;
    zstd->input.pos = 0;

    zstd->output.dst = zstd->buffer_out;
    zstd->input.size = 0;
    zstd->input.pos = 0;

    return zstd;

error:
    if (zstd->ctx)
        ZSTD_freeDCtx(zstd->ctx);

    if (zstd->buffer_in)
        free(zstd->buffer_in);

    if (zstd->buffer_out)
        free(zstd->buffer_out);

    free(zstd);

    return NULL;
}

int dec_zstd_stream(dec_zstd_t *zstd, dec_read_callback read, void *opaque, void **data, size_t *size_dec) {
    size_t ret;

    if (zstd->input.size == zstd->input.pos) {
        const int ret =
            read(opaque, zstd->buffer_in_size, zstd->buffer_in, &zstd->input.size);

        if (ret != DEC_OK)
            return ret;

        zstd->input.pos = 0;

    }

    zstd->output.size = zstd->buffer_out_size;
    zstd->output.pos = 0;

    ret = ZSTD_decompressStream(zstd->ctx, &zstd->output, &zstd->input);

    if (ZSTD_isError(ret)) {
        fprintf(stderr, "ZSTD_decompressStream() failed: %s\n", ZSTD_getErrorName(ret));
        return DEC_ERROR;
    }

    *data = zstd->output.dst;
    *size_dec = zstd->output.pos;

    return DEC_OK;
}

void dec_zstd_free(dec_zstd_t *zstd) {
    ZSTD_freeDCtx(zstd->ctx);

    free(zstd->buffer_in);
    free(zstd->buffer_out);
    free(zstd);
}

