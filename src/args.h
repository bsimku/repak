#ifndef ARGS_H
#define ARGS_H

typedef struct {
    enum {
        COMPRESS,
        RECOMPRESS,
        EXTRACT,
        NONE
    } action;

    int comp_level;
    char *comp_algorithm;
    char *target;
    char *output;
} args_t;

int args_parse(int argc, char *argv[], args_t *args);

#endif
