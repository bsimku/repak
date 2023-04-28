#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

void *safe_alloc(size_t size);
char *make_path(const char *directory, const char *name);

#endif
