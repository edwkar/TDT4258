#ifndef __UTILS__H
#define __UTILS__H

#include <stdio.h>
#include <stdlib.h>

#define die_hard(s) do { perror(s); exit(EXIT_FAILURE); } while (0)

void initialize_subsystems(void);
void cleanup_subsystems(void);
void * malloc_or_die(size_t s);

#define max(a, b) ((a)>(b) ? (a) : (b))
#define min(a, b) ((a)<(b) ? (a) : (b))
#define clamp(a, x, b) ((x) > (b) ? (b) : (x) < (a) ? (a) : (x))

#endif

#if 0
TODO
unsigned int time_millis(void) {
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return (unsigned int) ((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000);
}
#endif 

