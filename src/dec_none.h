#ifndef DEC_NONE_H
#define DEC_NONE_H

#include "dec_common.h"

typedef struct {
    void *buffer;
} dec_none_t;

dec_none_t *dec_none_init();
int dec_none_stream(dec_none_t *none, dec_read_callback read, void *opaque, void **data, size_t *size_dec);
void dec_none_free(dec_none_t *none);

#endif
