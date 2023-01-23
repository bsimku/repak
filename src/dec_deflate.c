#include "dec_deflate.h"

#include <stdlib.h>

#define IN_BUFFER_SIZE  128
#define OUT_BUFFER_SIZE 131072

dec_deflate_t *dec_deflate_init() {
    dec_deflate_t *deflate = malloc(sizeof(dec_deflate_t));

    if (!deflate)
        return NULL;

    deflate->in_buffer = NULL;
    deflate->out_buffer = NULL;

    if (!(deflate->in_buffer = malloc(IN_BUFFER_SIZE)))
        goto error;

    if (!(deflate->out_buffer = malloc(OUT_BUFFER_SIZE)))
        goto error;

    deflate->stream.zalloc = Z_NULL;
    deflate->stream.zfree = Z_NULL;
    deflate->stream.opaque = Z_NULL;
    deflate->stream.avail_in = 0;
    deflate->stream.next_in = Z_NULL;

    int ret = inflateInit2(&deflate->stream, -15);

    if (ret != Z_OK) {
        fprintf(stderr, "inflateInit() failed: %d\n", ret);
        goto error;
    }

    return deflate;

error:
    if (deflate->in_buffer)
        free(deflate->in_buffer);

    if (deflate->out_buffer)
        free(deflate->out_buffer);

    free(deflate);

    return NULL;
}

int dec_deflate_stream(dec_deflate_t *deflate, dec_read_callback read, void *opaque, void **data, size_t *size_dec) {
    if (deflate->stream.avail_in == 0) {
        size_t read_size;

        const int ret = read(opaque, IN_BUFFER_SIZE, deflate->in_buffer, &read_size);

        if (ret != DEC_OK)
            return ret;

        deflate->stream.avail_in = read_size;
        deflate->stream.next_in = deflate->in_buffer;
    }

    deflate->stream.avail_out = OUT_BUFFER_SIZE;
    deflate->stream.next_out = deflate->out_buffer;

    const int ret = inflate(&deflate->stream, Z_NO_FLUSH);

    *size_dec = OUT_BUFFER_SIZE - deflate->stream.avail_out;
    *data = deflate->out_buffer;

    if (ret == Z_STREAM_END) {
        inflateReset(&deflate->stream);
        return DEC_OK;
    }

    if (ret != Z_OK) {
        fprintf(stderr, "inflate() failed: %d", ret);
        return DEC_ERROR;
    }

    return DEC_OK;
}

void dec_deflate_free(dec_deflate_t *deflate) {
    inflateEnd(&deflate->stream);

    free(deflate->in_buffer);
    free(deflate->out_buffer);
    free(deflate);
}

