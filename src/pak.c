#include "pak.h"

#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "comp.h"

#define PAK_MAGIC 0x414b504b // "KPKA"

pak_t *pak_open(const char *filename) {
    pak_t *pak = malloc(sizeof(pak_t));

    if (!pak)
        return NULL;

    pak->dec_ctx = NULL;
    pak->comp_ctx = NULL;

    pak->file = fopen(filename, "r");

    if (pak->file == NULL) {
        fprintf(stderr, "failed to open '%s': %s\n", filename, strerror(errno));
        goto error;
    }

    if (fread(&pak->header, sizeof(pak_header_t), 1, pak->file) != 1) {
        fprintf(stderr, "failed to read header.\n");
        goto header_error;
    }

    if (pak->header.magic != PAK_MAGIC) {
        fprintf(stderr, "failed to read header: invalid magic.\n");
        goto header_error;
    }

    pak->files = malloc(sizeof(pak_file_t) * pak->header.file_count);

    if (!pak->files)
        return NULL;

    if (fread(pak->files, sizeof(pak_file_t), pak->header.file_count, pak->file) != pak->header.file_count) {
        fprintf(stderr, "failed to read file metadata.\n");
        goto files_error;
    }

    return pak;

files_error:
    free(pak->files);
header_error:
    fclose(pak->file);
error:
    free(pak);
    return NULL;
}

typedef struct {
    pak_t *pak;
    pak_file_t *file;
} pak_read_context_t;

size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

int pak_read_callback(void *opaque, size_t buffer_size, void *data, size_t *data_size) {
    pak_read_context_t *ctx = opaque;

    const size_t bytes_left =
        ctx->file->size_compressed + ctx->file->offset - ftell(ctx->pak->file);

    const size_t read_size = min(bytes_left, buffer_size);

    if (read_size == 0)
        return DEC_EOF;

    if ((*data_size = fread(data, 1, read_size, ctx->pak->file)) < read_size) {
        if (feof(ctx->pak->file))
            return DEC_EOF;

        fprintf(stderr, "fread() failed: %s\n", strerror(ferror(ctx->pak->file)));

        return DEC_ERROR;
    }

    return DEC_OK;
}

size_t pak_read(pak_t *pak, pak_file_t *file, comp_options_t *options, FILE *out_file) {
    int ret;

    if ((ret = fseek(pak->file, file->offset, SEEK_SET)) < 0) {
        fprintf(stderr, "fseek() failed: %s", strerror(ret));
        return 0;
    }

    if (!pak->dec_ctx && !(pak->dec_ctx = dec_init()))
        return 0;

    if (!pak->comp_ctx && !(pak->comp_ctx = comp_init()))
        return 0;

    pak_read_context_t context = {
        .pak = pak,
        .file = file
    };

    size_t total_size = 0, total_size_compressed = 0;

    while (true) {
        void *data;
        size_t size;

        ret = dec_stream(pak->dec_ctx, pak_read_callback, &context, file->flags, &data, &size);

        if (ret == DEC_ERROR)
            return 0;

        if (ret == DEC_EOF)
            break;

        total_size += size;

        const int flags = total_size == file->size ? COMP_FLAG_END : 0;

        const size_t ret = comp_stream(pak->comp_ctx, options, data, size, out_file, flags);

        if (ret == COMP_ERROR)
            return 0;

        total_size_compressed += ret;
    }

    if (total_size != file->size) {
        fprintf(stderr, "failed reading file: size mismatch.\n");
        return 0;
    }

    return total_size_compressed;
}

void pak_close(pak_t *pak) {
    fclose(pak->file);

    if (pak->dec_ctx) {
        dec_free(pak->dec_ctx);
    }

    if (pak->comp_ctx) {
        comp_free(pak->comp_ctx);
    }

    free(pak->files);
    free(pak);
}
