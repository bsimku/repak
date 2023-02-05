#ifndef REPACK_H
#define REPACK_H

#include <stdbool.h>

#include "comp.h"
#include "pak.h"

bool repack_from_file(const char *input, const char *output, comp_options_t *options);

#endif
