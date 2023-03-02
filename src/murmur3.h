#ifndef MURMUR3_H
#define MURMUR3_H

#include <stddef.h>
#include <stdint.h>

uint32_t murmur3_32(const char *key, size_t length);

#endif
