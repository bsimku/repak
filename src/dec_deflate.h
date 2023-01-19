#ifndef DEC_DEFLATE_H
#define DEC_DEFLATE_H

#include <stdio.h>

#include <zlib.h>

#include "dec_common.h"

typedef struct {
    void *in_buffer;
    void *out_buffer;
    z_stream stream;
} dec_deflate_t;

dec_deflate_t *dec_deflate_init();
int dec_deflate_stream(dec_deflate_t *deflate, dec_read_callback read, void *opaque, void **data, size_t *size_dec);
void dec_deflate_free(dec_deflate_t *deflate);

#endif
