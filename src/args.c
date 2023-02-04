#include "args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *Usage =
    "[option...] <target>\n"
    "Options:\n"
    "  -h, --help                   show this help message and exit.\n"
    "  -x, --extract                extract files from TARGET to OUTPUT.\n"
    "  -c, --compress <algorithm>   use specified compression algorithm (none, zstd, deflate).\n"
    "  -l, --compress-level <level> compression level.\n"
    "  -o, --output <output>        output file or directory.\n"
    "  -r, --repack                 repack TARGET and write to OUTPUT.\n";

const struct option Options[] = {
    {"help", no_argument, NULL, 'h'},
    {"extract", no_argument, NULL, 'x'},
    {"compression", required_argument, NULL, 'c'},
    {"compression-level", required_argument, NULL, 'l'},
    {"output", required_argument, NULL, 'o'},
    {"repack", no_argument, NULL, 'r'},
    {}
};

void args_print_usage(const char *exec) {
    fprintf(stderr, "Usage: %s %s", exec, Usage);
}

int args_parse_comp_algorithm(const char *algorithm, compress_type_e *type) {
    if (strcmp(algorithm, "zstd") == 0) {
        *type = COMPRESS_TYPE_ZSTD;
    }
    else if (strcmp(algorithm, "deflate") == 0) {
        *type = COMPRESS_TYPE_DEFLATE;
    }
    else if (strcmp(algorithm, "none") == 0) {
        *type = COMPRESS_TYPE_NONE;
    }
    else {
        fprintf(stderr, "Invalid compression algorithm provided.\n");
        return -1;
    }

    return 0;
}

int args_parse(int argc, char *argv[], args_t *args) {
    int ch, optionIndex = 0;

    args->action = ACTION_NONE;
    args->comp_options.type = COMPRESS_TYPE_NONE;
    args->comp_options.threads = 1;

    while ((ch = getopt_long(argc, argv, "hxc:l:o:r", Options, &optionIndex)) != -1) {
        switch (ch) {
            default:
                args_print_usage(argv[0]);
                return -1;
            case 'h':
                args_print_usage(argv[0]);
                return 0;
            case 'x':
                args->action = ACTION_UNPACK;
                break;
            case 'c':
                if (args_parse_comp_algorithm(optarg, &args->comp_options.type) < 0) {
                    args_print_usage(argv[0]);
                    return -1;
                }

                break;
            case 'l':
                args->comp_options.level = atoi(optarg);
                break;
            case 'o':
                args->output = optarg;
                break;
            case 'r':
                args->action = ACTION_REPACK;
                break;
        }
    }

    if (optind + 1 != argc) {
        args_print_usage(argv[0]);
        return -1;
    }

    args->target = argv[optind++];

    return 0;
}
