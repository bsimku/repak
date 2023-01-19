#include "dec_none.h"

#include <stdlib.h>

#define BUFFER_SIZE 131072

dec_none_t *dec_none_init() {
    dec_none_t *none = malloc(sizeof(dec_none_t));

    if (!none)
        return NULL;

    if (!(none->buffer = malloc(BUFFER_SIZE)))
        goto error;

    return none;

error:
    free(none);
    return NULL;
}

int dec_none_stream(dec_none_t *none, dec_read_callback read, void *opaque, void **data, size_t *size_dec) {
    *data = none->buffer;

    return read(opaque, BUFFER_SIZE, *data, size_dec);
}

void dec_none_free(dec_none_t *none) {
    free(none->buffer);
    free(none);
}

