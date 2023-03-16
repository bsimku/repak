#ifndef PAK_H
#define PAK_H

#include <stdio.h>
#include <stdint.h>

#include "comp.h"
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
    uint32_t hash_name_lower;
    uint32_t hash_name_upper;
    uint64_t offset;
    uint64_t size_compressed;
    uint64_t size;
    uint64_t flags;
    uint32_t hash;
    uint32_t unknown;
} pak_file_t;

typedef struct {
    FILE *file;
    pak_header_t header;
    pak_file_t *files;
    size_t file_idx;
    comp_ctx_t *comp_ctx;
    dec_ctx_t *dec_ctx;
} pak_t;

pak_t *pak_new(const char *filename);
pak_t *pak_open(const char *filename);
void pak_set_compression_flags(pak_file_t *file, const compress_type_e type);
bool pak_set_file_count(pak_t *pak, const size_t count);
bool pak_add_file(pak_t *pak, const char *filename, comp_options_t *options);
bool pak_write_metadata(pak_t *pak);
size_t pak_read(pak_t *pak, pak_file_t *file, comp_options_t *options, FILE *out_file);
void pak_close(pak_t *pak);

#endif
