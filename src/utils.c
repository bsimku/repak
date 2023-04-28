#include "utils.h"

#include <stdlib.h>
#include <string.h>

void *safe_alloc(size_t size) {
    void *ptr = malloc(size);

    if (!ptr) {
        abort();
    }

    return ptr;
}

char *make_path(const char *directory, const char *name) {
    char *path = safe_alloc(strlen(directory) + strlen(name) + 2);

    strcpy(path, directory);
    strcat(path, "/");
    strcat(path, name);

    return path;
}

