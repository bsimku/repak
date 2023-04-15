#include "pack.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "pak.h"

static char *make_path(const char *directory, const char *name) {
    char *path = malloc(strlen(directory) + strlen(name) + 2);

    if (!path)
        return NULL;

    strcpy(path, directory);
    strcat(path, "/");
    strcat(path, name);

    return path;
}

static bool count_files(const char *directory, size_t *count) {
    DIR *dir = opendir(directory);

    if (!dir) {
        fprintf(stderr, "opendir() failed: %s", strerror(errno));
        return false;
    }

    struct dirent *ent;

    while ((ent = readdir(dir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;

        char *path = make_path(directory, ent->d_name);

        if (!path)
            goto error;

        if (ent->d_type != DT_DIR) {
            (*count)++;
        }

        if (ent->d_type == DT_DIR && !count_files(path, count)) {
            free(path);
            goto error;
        }

        free(path);
    }

    closedir(dir);

    return true;

error:
    closedir(dir);
    return false;
}

static bool traverse_add_files(pak_t *pak, const char *directory, comp_options_t *options) {
    DIR *dir = opendir(directory);

    if (!dir) {
        fprintf(stderr, "opendir() failed: %s", strerror(errno));
        return false;
    }

    struct dirent *ent;

    while ((ent = readdir(dir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;

        char *path = make_path(directory, ent->d_name);

        if (!path)
            return false;

        if (ent->d_type != DT_DIR) {
            pak_add_file(pak, path, options);
        }

        if (ent->d_type == DT_DIR && !traverse_add_files(pak, path, options)) {
            free(path);
            return false;
        }

        free(path);
    }

    return true;
}

bool pack_from_directory(const char *directory, const char *output, comp_options_t *options) {
    pak_t *pak = pak_new(output);

    if (!pak)
        return false;

    size_t file_count = 0;

    if (!count_files(directory, &file_count))
        goto error;

    if (!pak_set_file_count(pak, file_count))
        goto error;

    if (!traverse_add_files(pak, directory, options))
        goto error;

    if (!pak_write_metadata(pak))
        goto error;

    pak_close(pak);

    return true;

error:
    pak_close(pak);

    return false;
}
