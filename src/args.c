#include "args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *Usage =
    "[option...] <target>\n"
    "Options:\n"
    "  -h, --help                   show this help message and exit.\n"
    "  -p, --pack                   pack files from TARGET and write to OUTPUT.\n"
    "  -r, --repack                 repack TARGET and write to OUTPUT.\n"
    "  -x, --extract                extract files from TARGET to OUTPUT.\n"
    "  -c, --compress <algorithm>   use specified compression algorithm (none, zstd, deflate).\n"
    "  -l, --compress-level <level> compression level.\n"
    "  -i, --file-list <filename>   file list for unpacking.\n"
    "  -t, --threads <threads>      number of threads for compression.\n"
    "  -o, --output <output>        output file or directory.\n";

const struct option Options[] = {
    {"help", no_argument, NULL, 'h'},
    {"extract", no_argument, NULL, 'x'},
    {"compression", required_argument, NULL, 'c'},
    {"compression-level", required_argument, NULL, 'l'},
    {"file-list", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"pack", no_argument, NULL, 'p'},
    {"repack", no_argument, NULL, 'r'},
    {"threads", no_argument, NULL, 't'},
    {}
};

static void print_usage(const char *exec) {
    fprintf(stderr, "Usage: %s %s", exec, Usage);
}

static bool parse_comp_algorithm(const char *algorithm, compress_type_e *type) {
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
        return false;
    }

    return true;
}

static bool validate(args_t *args) {
    if (args->action != ACTION_NONE && args->output == NULL) {
        fprintf(stderr, "Option --output is required for this action.\n");
        return false;
    }

    if (args->action == ACTION_UNPACK && !args->file_list) {
        fprintf(stderr, "--file-list is required for --unpack.\n");
        return false;
    }

    return true;
}

bool args_parse(int argc, char *argv[], args_t *args) {
    int ch, optionIndex = 0;

    args->action = ACTION_NONE;
    args->comp_options.type = COMPRESS_TYPE_NONE;
    args->comp_options.threads = 1;
    args->file_list = NULL;
    args->target = NULL;
    args->output = NULL;

    while ((ch = getopt_long(argc, argv, "hxc:l:i:o:rt:", Options, &optionIndex)) != -1) {
        switch (ch) {
            default:
                print_usage(argv[0]);
                return false;
            case 'h':
                print_usage(argv[0]);
                return true;
            case 'x':
                args->action = ACTION_UNPACK;
                break;
            case 'c':
                if (!parse_comp_algorithm(optarg, &args->comp_options.type)) {
                    print_usage(argv[0]);
                    return false;
                }

                break;
            case 'l':
                args->comp_options.level = atoi(optarg);
                break;
            case 'i':
                args->file_list = optarg;
                break;
            case 'o':
                args->output = optarg;
                break;
            case 'p':
                args->action = ACTION_PACK;
                break;
            case 'r':
                args->action = ACTION_REPACK;
                break;
            case 't':
                args->comp_options.threads = atoi(optarg);
                break;
        }
    }

    if (optind + 1 != argc) {
        print_usage(argv[0]);
        return false;
    }

    args->target = argv[optind++];

    if (!validate(args)) {
        print_usage(argv[0]);
        return false;
    }

    return true;
}
