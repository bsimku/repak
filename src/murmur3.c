#include "murmur3.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>

#include "stdio.h"

static uint32_t rotl32(uint32_t x, uint32_t n) {
    return (x << n) | (x >> (32 - n));
}

static uint32_t murmur3_32_scramble(uint32_t k) {
    k = k * 0xcc9e2d51;
    k = rotl32(k, 15);
    k = k * 0x1b873593;

    return k;
}

static uint32_t murmur3_32_finalize(uint32_t hash) {
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

static uint32_t murmur3_32(const char *key, size_t length, bool upper) {
    uint32_t hash = 0xffffffff;

    for (size_t i = 0; i < length - 1; i += 2) {
        uint32_t k = upper ? (toupper(key[i]) | toupper(key[i + 1]) << 16)
            : (key[i] | key[i + 1] << 16);

        hash ^= murmur3_32_scramble(k);
        hash = rotl32(hash, 13);
        hash = hash * 5 + 0xe6546b64;
    }

    if (length % 2) {
        hash ^= murmur3_32_scramble(upper ? toupper(key[length - 1]) : key[length - 1]);
    }

    hash ^= length * 2;

    return murmur3_32_finalize(hash);
}

uint32_t murmur3(const char *key, size_t length) {
    return murmur3_32(key, length, false);
}

uint32_t murmur3_upper(const char *key, size_t length) {
    return murmur3_32(key, length, true);
}

