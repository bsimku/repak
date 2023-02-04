#ifndef REPACK_H
#define REPACK_H

#include "comp.h"
#include "pak.h"

int repack_from_file(const char *input, const char *output, comp_options_t *options);

#endif
