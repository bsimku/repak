#ifndef DEC_COMMON_H
#define DEC_COMMON_H

#include <stddef.h>

#define DEC_EOF    1
#define DEC_OK     0
#define DEC_ERROR -1

typedef int (*dec_read_callback)(void *, size_t, void *, size_t *);

#endif
