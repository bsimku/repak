#ifndef PACK_H
#define PACK_H

#include <stdbool.h>

#include "comp_common.h"

bool pack_from_directory(const char *directory, const char *output, comp_options_t *options);

#endif
