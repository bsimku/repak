#include "dec_none.h"

#include <stdlib.h>

#include "utils.h"

#define BUFFER_SIZE 131072

dec_none_t *dec_none_init() {
    dec_none_t *none = safe_alloc(sizeof(dec_none_t));

    none->buffer = safe_alloc(BUFFER_SIZE);

    return none;
}

int dec_none_stream(dec_none_t *none, dec_read_callback read, void *opaque, void **data, size_t *size_dec) {
    *data = none->buffer;

    return read(opaque, BUFFER_SIZE, *data, size_dec);
}

void dec_none_free(dec_none_t *none) {
    free(none->buffer);
    free(none);
}
