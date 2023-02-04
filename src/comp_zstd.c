#include "comp_zstd.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool comp_zstd_set_parameter(ZSTD_CCtx *ctx, ZSTD_cParameter parameter, int value) {
    const int ret = ZSTD_CCtx_setParameter(ctx, parameter, value);

    if (ZSTD_isError(ret)) {
        fprintf(stderr, "ZSTD_CCtx_setParameter() failed: %s\n", ZSTD_getErrorName(ret));
        return false;
    }

    return true;
}

comp_zstd_t *comp_zstd_init(comp_options_t *options) {
    comp_zstd_t *zstd = malloc(sizeof(comp_zstd_t));

    if (!zstd)
        return NULL;

    if (!(zstd->ctx = ZSTD_createCCtx()))
        goto ctx_error;

    zstd->buffer_out = NULL;
    zstd->buffer_out_size = ZSTD_DStreamOutSize();

    if (!(zstd->buffer_out = malloc(zstd->buffer_out_size)))
        goto error;

    zstd->output.dst = zstd->buffer_out;
    zstd->output.size = zstd->buffer_out_size;
    zstd->output.pos = 0;

    if (!comp_zstd_set_parameter(zstd->ctx, ZSTD_c_compressionLevel, options->level))
        goto error;

    if (!comp_zstd_set_parameter(zstd->ctx, ZSTD_c_nbWorkers, options->threads))
        goto error;

    return zstd;

error:
    ZSTD_freeCCtx(zstd->ctx);
ctx_error:
    free(zstd);

    return NULL;
}

size_t comp_zstd_stream(comp_zstd_t *zstd, void *data, size_t size, FILE *out_file, int flags) {
    const ZSTD_EndDirective mode = flags & COMP_FLAG_END ? ZSTD_e_end : ZSTD_e_continue;

    zstd->input.src = data;
    zstd->input.size = size;
    zstd->input.pos = 0;

    size_t size_compressed = 0;

    while (zstd->input.pos < zstd->input.size) {
        zstd->output.pos = 0;

        const size_t remaining = ZSTD_compressStream2(zstd->ctx, &zstd->output, &zstd->input, mode);

        if (ZSTD_isError(remaining)) {
            fprintf(stderr, "ZSTD_compressStream2() failed: %s", ZSTD_getErrorName(remaining));
            return COMP_ERROR;
        }

        const int ret = fwrite(zstd->output.dst, zstd->output.pos, 1, out_file);

        if (ret < 0) {
            fprintf(stderr, "fwrite() failed: %s\n", strerror(ret));
            return COMP_ERROR;
        }

        size_compressed += zstd->output.pos;
    }

    return size_compressed;
}

void comp_zstd_free(comp_zstd_t *zstd) {
    ZSTD_freeCCtx(zstd->ctx);

    free(zstd->buffer_out);
    free(zstd);
}
