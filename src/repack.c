#include "repack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pak_flags.h"

int comp(const void *e1, const void *e2) {
    const pak_file_t *a = *(const pak_file_t **)e1;
    const pak_file_t *b = *(const pak_file_t **)e2;

    const int ret = (a->offset > b->offset) - (a->offset < b->offset);

    if (ret != 0)
        return ret;

    return (a->flags > b->flags) - (a->flags < b->flags);
}

bool seek(FILE *out_file, size_t offset) {
    const int ret = fseek(out_file, offset, SEEK_SET);

    if (ret < 0) {
        fprintf(stderr, "fseek() failed: %s\n", strerror(ret));
        return false;
    }

    return true;
}

void set_compression_flags(pak_file_t *file, compress_type_e type) {
    file->flags = file->flags & ~PAK_FILE_FLAG_DEFLATE & ~PAK_FILE_FLAG_ZSTD;

    switch (type) {
        case COMPRESS_TYPE_NONE:
            break;
        case COMPRESS_TYPE_DEFLATE:
            file->flags &= PAK_FILE_FLAG_DEFLATE;
            break;
        case COMPRESS_TYPE_ZSTD:
            file->flags &= PAK_FILE_FLAG_ZSTD;
            break;
    }
}

bool write_data(FILE *out_file, pak_t *pak, comp_options_t *options) {
    const size_t header_size = sizeof(pak_header_t);
    const size_t files_size = sizeof(pak_file_t) * pak->header.file_count;

    if (!seek(out_file, header_size + files_size))
        return false;

    pak_file_t **files = malloc(files_size);

    for (int i = 0; i < pak->header.file_count; i++) {
        files[i] = &pak->files[i];
    }

    qsort(files, pak->header.file_count, sizeof(pak_file_t *), comp);

    for (int i = 0; i < pak->header.file_count;) {
        pak_file_t *file = files[i];

        size_t size;

        if ((size = pak_read(pak, file, options, out_file)) == 0)
            return false;

        for (; i < pak->header.file_count; i++) {
            pak_file_t *other = files[i];

            if (other->hash != file->hash)
                break;

            set_compression_flags(other, options->type);

            other->offset = ftell(out_file) - size;
            other->size_compressed = size;
        }
    }

    return true;
}

int write_metadata(FILE *out_file, pak_t *pak) {
    if (!seek(out_file, 0))
        return false;

    const size_t header_size = sizeof(pak_header_t);
    const size_t files_size = sizeof(pak_file_t) * pak->header.file_count;

    int ret;

    if ((ret = fwrite(&pak->header, 1, header_size, out_file)) < 0)
        goto fwrite_error;

    if ((ret = fwrite(pak->files, 1, files_size, out_file)) < 0)
        goto fwrite_error;

    return true;

fwrite_error:
        fprintf(stderr, "fwrite() failed: %s\n", strerror(ret));
        return -1;
}

bool repack_from_file(const char *input, const char *output, comp_options_t *options) {
    bool ret = true;

    pak_t *pak = pak_open(input);

    if (!pak)
        goto error;

    FILE *out_file = fopen(output, "w");

    if (!out_file) {
        fprintf(stderr, "failed to open file '%s' for writing.\n", output);
        goto error;
    }

    if (!write_data(out_file, pak, options))
        goto error;

    if (!write_metadata(out_file, pak))
        goto error;

cleanup:
    if (pak) {
        pak_close(pak);
    }

    if (out_file) {
        fclose(out_file);
    }

    return ret;
error:
    ret = false;
    goto cleanup;
}
