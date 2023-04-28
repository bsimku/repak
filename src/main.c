#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "pack.h"
#include "pak.h"
#include "repack.h"
#include "unpack.h"

#include "murmur3.h"

int main(int argc, char *argv[]) {
    args_t args;

    if (!args_parse(argc, argv, &args) || args.action == ACTION_NONE)
        return 1;

    if (args.action == ACTION_REPACK && !repack_from_file(args.target, args.output, &args.comp_options))
        return 1;

    else if (args.action == ACTION_PACK && !pack_from_directory(args.target, args.output, &args.comp_options))
       return 1;

    else if (args.action == ACTION_UNPACK && !unpack_files(args.target, args.output, args.file_list))
        return 1;

    return 0;
}
