#include "args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

const char *Usage =
    "[option...] <target>\n"
    "Options:\n"
    "  -h, --help                   show this help message and exit.\n"
    "  -x, --extract                extract files from TARGET to OUTPUT.\n"
    "  -c, --compress <algorithm>   use specified compression algorithm (none, zstd, deflate).\n"
    "  -l, --compress-level <level> compression level.\n"
    "  -o, --output <output>        output file or directory.\n"
    "  -r, --recompress             recompress TARGET and write to OUTPUT.\n";

const struct option Options[] = {
    {"help", no_argument, NULL, 'h'},
    {"extract", no_argument, NULL, 'x'},
    {"compression", required_argument, NULL, 'c'},
    {"compression-level", required_argument, NULL, 'l'},
    {"output", required_argument, NULL, 'o'},
    {"recompress", no_argument, NULL, 'r'},
    {}
};

void args_print_usage(const char *exec) {
    fprintf(stderr, "Usage: %s %s", exec, Usage);
}

int args_parse(int argc, char *argv[], args_t *args) {
    int ch, optionIndex = 0;

    args->action = NONE;

    while ((ch = getopt_long(argc, argv, "hxc:l:o:r", Options, &optionIndex)) != -1) {
        switch (ch) {
            default:
                args_print_usage(argv[0]);
                return -1;
            case 'h':
                args_print_usage(argv[0]);
                return 0;
            case 'x':
                args->action = UNPACK;
                break;
            case 'c':
                args->comp_algorithm = optarg;
                break;
            case 'l':
                args->comp_level = atoi(optarg);
                break;
            case 'o':
                args->output = optarg;
                break;
            case 'r':
                args->action = REPACK;
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
