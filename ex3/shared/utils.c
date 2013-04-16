#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "audio.h"
#include "input.h"
#include "screen.h"
#include "utils.h"


void * malloc_or_die(size_t s)
{
    void *v = malloc(s);

    if (v == NULL)
        DIE_HARD("malloc");

    return v;
}

int open_or_die(const char *pathname, int flags)
{
    int fd = open(pathname, flags);

    if (fd < 0) {
        fprintf(stderr, "Failed to open() %s. Panicking. \n", pathname);
        exit(EXIT_FAILURE);
    }

    return fd;
}

FILE * fopen_or_die(const char *path, const char *mode)
{
    FILE * f = fopen(path, mode);

    if (f == NULL) {
        fprintf(stderr, "Failed to fopen() %s. Panicking. \n", path);
        exit(EXIT_FAILURE);
    }

    return f;
}
