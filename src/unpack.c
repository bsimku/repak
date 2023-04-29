#include "unpack.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include "pak.h"
#include "murmur3.h"
#include "utils.h"

struct file_entry {
    char *path;
    uint32_t hash;
};

struct file_list {
    size_t length;
    struct file_entry *entries;
};

static struct file_list *read_file_list(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "failed to open '%s': %s\n", filename, strerror(errno));
        return NULL;
    }

    size_t entry_count = 0;

    while (!feof(file)) {
        if (fgetc(file) == '\n') {
            entry_count++;
        }
    }

    rewind(file);

    struct file_entry *entries = safe_alloc(sizeof(struct file_entry) * entry_count);

    char path[PATH_MAX];
    size_t path_length = 0, entry_idx = 0;

    for (char ch; !feof(file); ch = fgetc(file)) {
        if (ch == '\0')
            continue;

        if (ch == '\n') {
            path[path_length++] = '\0';

            entries[entry_idx].hash = murmur3(path, path_length - 1);
            entries[entry_idx].path = safe_alloc(path_length);

            memcpy(entries[entry_idx].path, path, path_length);

            entry_idx++;
            path_length = 0;
        }
        else {
            if (path_length < PATH_MAX) {
                path[path_length++] = ch;
            }
            else {
                fprintf(stderr, "error reading '%s': file path entry too long.\n", filename);
                goto error;
            }
        }
    }

    struct file_list *list = safe_alloc(sizeof(struct file_list));

    list->length = entry_count;
    list->entries = entries;

    return list;

error:
    for (size_t i = 0; i < entry_idx; i++) {
        free(entries[i].path);
    }

    free(entries);

    fclose(file);

    return NULL;
}

static void free_file_list(struct file_list *list) {
    for (size_t i = 0; i < list->length; i++) {
        free(list->entries[i].path);
    }

    free(list);
}

static char *get_dirname(const char *path) {
    const char *last_separator = NULL;

    for (const char *p = path; *p; p++) {
        if (*p == '/') {
            last_separator = p;
        }
    }

    if (!last_separator)
        return NULL;

    const size_t length = last_separator - path;

    char *dir = safe_alloc(length + 1);

    memcpy(dir, path, length);
    dir[length] = '\0';

    return dir;
}

static bool create_dir(const char *directory) {
    if (mkdir(directory, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "failed to create directory '%s': %s\n", directory, strerror(errno));
        return false;
    }

    return true;
}

static bool create_dirs(const char *directory) {
    if (*directory == '\0')
        return false;

    char *dir = strdup(directory);

    for (char *p = dir + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';

            if (!create_dir(dir))
                goto error;

            *p = '/';
        }
    }

    free(dir);

    if (!create_dir(directory))
        return false;

    return true;

error:
    free(dir);

    return false;
}

static bool unpack_file(pak_t *pak, pak_file_t *file, const char *path, const char *output_dir) {
    char *dirname = get_dirname(path);

    if (!dirname)
        return false;

    char *out_dir = make_path(output_dir, dirname);

    free(dirname);

    if (!out_dir)
        return false;

    const bool ret = create_dirs(out_dir);

    free(out_dir);

    if (!ret)
        return false;

    char *out_path = make_path(output_dir, path);

    if (!out_path)
        return false;

    FILE *out_file = fopen(out_path, "wb");

    if (!out_file) {
        fprintf(stderr, "failed to open file '%s': %s", out_path, strerror(errno));
        free(out_path);
        return false;
    }

    free(out_path);

    comp_options_t options = {
        .type = COMPRESS_TYPE_NONE
    };

    if (pak_read(pak, file, &options, out_file) == COMP_ERROR) {
        fclose(out_file);
        return false;
    }

    fclose(out_file);

    return true;
}

bool unpack_files(const char *input, const char *output_dir, const char *file_list) {
    struct file_list *list = read_file_list(file_list);

    if (!list)
        return false;

    pak_t *pak = pak_open(input);

    if (!pak)
        goto error;

    for (int i = 0; i < pak->header.file_count; i++) {
        pak_file_t *file = &pak->files[i];

        bool found = false;

        for (size_t i = 0; i < list->length; i++) {
            if (list->entries[i].hash != file->hash_name_lower)
                continue;

            fprintf(stderr, "%s %u\n", list->entries[i].path, file->size);

            if (!unpack_file(pak, file, list->entries[i].path, output_dir))
                goto error;

            found = true;
        }

        if (!found) {
            printf("hash not found: %u\n", file->hash_name_lower);
        }
    }

    pak_close(pak);
    free_file_list(list);

    return true;

error:
    free(list);
    return false;
}
