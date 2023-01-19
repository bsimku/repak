#ifndef PAK_H
#define PAK_H

#include <stdio.h>
#include <stdint.h>

#include "dec.h"

typedef struct pak_header_t {
    uint32_t magic;
    uint8_t  version_major;
    uint8_t  version_minor;
    uint16_t flags;
    uint32_t file_count;
    uint32_t hash;
} pak_header_t;

typedef struct {
    uint64_t hash;
    uint64_t offset;
    uint64_t size_compressed;
    uint64_t size;
    uint64_t compression_type;
    uint64_t dependency_hash;
} pak_file_t;

typedef struct {
    FILE *file;
    pak_header_t header;
    pak_file_t *files;
    dec_ctx_t *dec_ctx;
} pak_t;

pak_t *pak_open(const char *filename);
size_t pak_read(pak_t *pak, pak_file_t *file, FILE *out_file);
void pak_close(pak_t *pak);

#endif
