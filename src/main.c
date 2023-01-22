#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "pak.h"

int main(int argc, char *argv[]) {
    args_t args;

    const int ret = args_parse(argc, argv, &args);

    if (ret != 0)
        return ret;

    pak_t *pak = pak_open(args.target);

    if (!pak)
        return -1;

    if (args.action == RECOMPRESS) {
        FILE *out_file = fopen(args.output, "w");

        if (!out_file) {
            fprintf(stderr, "failed to open file '%s' for writing.\n", args.output);
            return -1;
        }

        const size_t header_size = sizeof(pak_header_t);
        const size_t files_size = sizeof(pak_file_t) * pak->header.file_count;

        int ret;

        if ((ret = fseek(out_file, header_size + files_size, SEEK_SET)) < 0)
            goto fseek_error;

        for (int i = 0; i < pak->header.file_count; i++) {
            pak_file_t *file = &pak->files[i];

            size_t size;

            if ((size = pak_read(pak, file, out_file)) == 0)
                return -1;

            file->offset = ftell(out_file) - size;
            file->size_compressed = size;
            file->compression_type = 0;
        }

        if ((ret = fseek(out_file, 0, SEEK_SET)) < 0)
            goto fseek_error;

        if ((ret = fwrite(&pak->header, 1, header_size, out_file)) < 0)
            goto fwrite_error;

        if ((ret = fwrite(pak->files, 1, files_size, out_file)) < 0)
            goto fwrite_error;

        fclose(out_file);

        return 0;

fseek_error:
        fprintf(stderr, "fseek() failed: %s\n", strerror(ret));
        return -1;
fwrite_error:
        fprintf(stderr, "fwrite() failed: %s\n", strerror(ret));
        return -1;
    }

    return 0;
}
