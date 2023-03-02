#include "murmur3.h"

#include <string.h>

uint32_t murmur3_32_scramble(uint32_t k) {
    k = k * 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k = k * 0x1b873593;

    return k;
}

uint32_t murmur3_32_finalize(uint32_t hash) {
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

uint32_t murmur3_32(const char *key, size_t length) {
    uint32_t hash = 0xffffffff;

    for (size_t i = 0; i < length / sizeof(uint32_t); i++) {
        uint32_t k;

        memcpy(&k, key + i * sizeof(uint32_t), sizeof(uint32_t));

        hash ^= murmur3_32_scramble(k);
        hash = (hash << 13) | (hash >> 19);
        hash = hash * 5 + 0xe6546b64;
    }

    uint32_t k = 0;

    for (int i = 0; i < length % sizeof(uint32_t); i++) {
        k <<= 8;
        k |= key[length - i - 1];
    }

    hash ^= murmur3_32_scramble(k);
    hash ^= length;

    return murmur3_32_finalize(hash);
}
