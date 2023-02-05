#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

#include "comp_common.h"

typedef struct {
    enum {
        ACTION_PACK,
        ACTION_UNPACK,
        ACTION_REPACK,
        ACTION_NONE
    } action;

    comp_options_t comp_options;

    char *target;
    char *output;
} args_t;

bool args_parse(int argc, char *argv[], args_t *args);

#endif
