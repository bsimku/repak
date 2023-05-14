#include "pak.h"

#include <errno.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "comp.h"
#include "murmur3.h"
#include "pak_flags.h"
#include "utils.h"

#define PAK_MAGIC 0x414b504b // "KPKA"
#define READ_BUFFER_SIZE 1048576

pak_t *pak_new(const char *filename) {
    pak_t *pak = safe_alloc(sizeof(pak_t));

    pak->files = NULL;
    pak->file_idx = 0;
    pak->comp_ctx = NULL;
    pak->dec_ctx = NULL;

    pak->header.magic = PAK_MAGIC;
    pak->header.version_major = 4;
    pak->header.version_minor = 0;
    pak->header.flags = 0;
    pak->header.file_count = 0;
    pak->header.hash = 0;

    pak->file = fopen(filename, "w");

    if (!pak->file) {
        fprintf(stderr, "failed to open '%s' for writing: %s\n", filename, strerror(errno));
        goto fopen_error;
    }

    return pak;

fopen_error:
    free(pak);

    return NULL;
}

pak_t *pak_open(const char *filename) {
    pak_t *pak = safe_alloc(sizeof(pak_t));

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

    pak->files = safe_alloc(sizeof(pak_file_t) * pak->header.file_count);

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

void pak_set_compression_flags(pak_file_t *file, const compress_type_e type) {
    file->flags = file->flags & ~PAK_FILE_FLAG_DEFLATE & ~PAK_FILE_FLAG_ZSTD;

    switch (type) {
        case COMPRESS_TYPE_NONE:
            break;
        case COMPRESS_TYPE_DEFLATE:
            file->flags |= PAK_FILE_FLAG_DEFLATE;
            break;
        case COMPRESS_TYPE_ZSTD:
            file->flags |= PAK_FILE_FLAG_ZSTD;
            break;
    }
}

bool pak_set_file_count(pak_t *pak, const size_t count) {
    const size_t size = sizeof(pak_file_t) * count;

    pak->files = safe_alloc(size);

    if (fseek(pak->file, sizeof(pak->header) + size, SEEK_SET)) {
        fprintf(stderr, "fseek() failed: %s", strerror(errno));
        return false;
    }

    pak->header.file_count = count;

    return true;
}

static bool write_callback(void *opaque, void *data, size_t size) {
    if (!size)
        return true;

    FILE *out_file = (FILE *)opaque;

    if (fwrite(data, size, 1, out_file) == 0) {
        fprintf(stderr, "fwrite() failed: %d\n", ferror(out_file));
        return false;
    }

    return true;
}

bool pak_add_file(pak_t *pak, const char *filename, comp_options_t *options) {
    if (!pak->dec_ctx && !(pak->dec_ctx = dec_init()))
        return false;

    if (!pak->comp_ctx && !(pak->comp_ctx = comp_init()))
        return false;

    struct stat st;

    if (stat(filename, &st) == -1) {
        fprintf(stderr, "failed to stat '%s': %s", filename, strerror(errno));
        return false;
    }

    if (!comp_set_size(pak->comp_ctx, options, st.st_size))
        return false;

    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "failed to open '%s' for reading: %s\n", filename, strerror(errno));
        return false;
    }

    char buf[READ_BUFFER_SIZE];

    size_t total_size = 0, total_size_comp = 0;

    while (true) {
        size_t ret = fread(buf, 1, READ_BUFFER_SIZE, file);

        if (!ret) {
            if (feof(file))
                break;

            return false;
        }

        const size_t size_comp = comp_stream(pak->comp_ctx, options, write_callback, pak->file, buf, ret);

        if (size_comp == COMP_ERROR)
            return false;

        total_size += ret;
        total_size_comp += size_comp;
    }

    pak_file_t *pak_file = &pak->files[pak->file_idx++];

    size_t len = strlen(filename);

    pak_file->hash_name_lower = murmur3(filename, len);
    pak_file->hash_name_upper = murmur3_upper(filename, len);
    pak_file->offset = ftell(pak->file) - total_size_comp;
    pak_file->size = total_size;
    pak_file->size_compressed = total_size_comp;
    pak_file->flags = 0;
    pak_file->hash = 0;
    pak_file->unknown = 0;

    pak_set_compression_flags(pak_file, options->type);

    return true;
}

bool pak_write_metadata(pak_t *pak) {
    rewind(pak->file);

    if (!fwrite(&pak->header, sizeof(pak->header), 1, pak->file)) {
        fprintf(stderr, "failed writing header: %s\n", strerror(ferror(pak->file)));
        return false;
    }

    if (!fwrite(pak->files, pak->header.file_count * sizeof(pak_file_t), 1, pak->file)) {
        fprintf(stderr, "failed writing file metadata: %s\n", strerror(ferror(pak->file)));
        return false;
    }

    return true;
}

typedef struct {
    pak_t *pak;
    pak_file_t *file;
} pak_read_context_t;

static size_t min(size_t a, size_t b) {
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

bool pak_read_prepare(pak_t *pak, pak_file_t *file, comp_options_t *options) {
    if (fseek(pak->file, file->offset, SEEK_SET)) {
        fprintf(stderr, "fseek() failed: %s", strerror(errno));
        return false;
    }

    if (!pak->dec_ctx && !(pak->dec_ctx = dec_init()))
        return false;

    if (!pak->comp_ctx && !(pak->comp_ctx = comp_init()))
        return false;

    if (!comp_set_size(pak->comp_ctx, options, file->size))
        return false;

    return true;
}

size_t pak_read(pak_t *pak, pak_file_t *file, comp_options_t *options, FILE *out_file) {
    if (!pak_read_prepare(pak, file, options))
        return 0;

    pak_read_context_t context = {
        .pak = pak,
        .file = file
    };

    size_t total_size = 0, total_size_compressed = 0;

    while (true) {
        void *data;
        size_t size;

        const size_t ret = dec_stream(pak->dec_ctx, pak_read_callback, &context, file->flags, &data, &size);

        if (ret == DEC_ERROR)
            return 0;

        if (ret == DEC_EOF)
            break;

        const size_t size_comp = comp_stream(pak->comp_ctx, options, write_callback, out_file, data, size);

        if (size_comp == COMP_ERROR)
            return 0;

        total_size += size;
        total_size_compressed += size_comp;
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
