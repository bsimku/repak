#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "pak.h"
#include "repack.h"

int main(int argc, char *argv[]) {
    args_t args;

    if (!args_parse(argc, argv, &args) || args.action == ACTION_NONE)
        return 1;

    if (args.action == ACTION_REPACK) {
        return repack_from_file(args.target, args.output, &args.comp_options);
    }

    return 0;
}
