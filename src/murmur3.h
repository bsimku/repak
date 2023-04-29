#ifndef MURMUR3_H
#define MURMUR3_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

uint32_t murmur3(const char *key, size_t length);
uint32_t murmur3_upper(const char *key, size_t length);

#endif
