#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "pak.h"
#include "repack.h"

int main(int argc, char *argv[]) {
    args_t args;

    const int ret = args_parse(argc, argv, &args);

    if (ret != 0 || args.action == ACTION_NONE)
        return ret;

    if (args.action == ACTION_REPACK) {
        return repack_from_file(args.target, args.output, &args.comp_options);
    }

    return 0;
}
