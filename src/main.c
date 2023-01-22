#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "pak.h"

const char *Usage =
    "[option...] <target>\n"
    "Options:\n"
    "  -h, --help                   show this help message and exit.\n"
    "  -x, --extract                extract files from TARGET to OUTPUT.\n"
    "  -c, --compress <algorithm>   use specified compression algorithm (none, zstd, deflate).\n"
    "  -l, --compress-level <level> compression level.\n"
    "  -o, --output <output>        output file or directory.\n"
    "  -r, --recompress             recompress FILENAME and write to OUTPUT.\n";

const struct option Options[] = {
    {"help", no_argument, NULL, 'h'},
    {"extract", no_argument, NULL, 'x'},
    {"compression", required_argument, NULL, 'c'},
    {"compression-level", required_argument, NULL, 'l'},
    {"output", required_argument, NULL, 'o'},
    {"recompress", no_argument, NULL, 'r'},
    {}
};

void print_usage(const char *exec) {
    fprintf(stderr, "Usage: %s %s", exec, Usage);
}

typedef enum {
    COMPRESS,
    RECOMPRESS,
    EXTRACT,
    NONE
} action_e;

int main(int argc, char *argv[]) {
    int ch, optionIndex = 0;

    char *comp_algorithm, *output;
    int comp_level;
    action_e action = NONE;

    while ((ch = getopt_long(argc, argv, "hxc:l:r", Options, &optionIndex)) != -1) {
        switch (ch) {
            default:
                print_usage(argv[0]);
                return -1;
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 'x':
                action = EXTRACT;
                break;
            case 'c':
                comp_algorithm = optarg;
                break;
            case 'l':
                comp_level = atoi(optarg);
                break;
            case 'o':
                output = optarg;
                break;
            case 'r':
                action = RECOMPRESS;
                break;
        }
    }

    if (optind + 1 != argc) {
        print_usage(argv[0]);
        return -1;
    }

    char *target = argv[optind++];

    pak_t *pak = pak_open(target);

    if (!pak)
        return -1;

    if (action == RECOMPRESS) {
        FILE *out_file = fopen(output, "w");

        if (!out_file) {
            fprintf(stderr, "failed to open file '%s' for writing.\n", output);
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
