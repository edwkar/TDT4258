#ifndef UTILSH
#define UTILSH

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


#define DIE_HARD(s) do { \
                        perror(__FILE__ ": " s);\
                        exit(EXIT_FAILURE);\
                    } while (0)

#define MAX(a, b)      ((a)>(b) ? (a) : (b))
#define MIN(a, b)      ((a)<(b) ? (a) : (b))
#define CLAMP(a, x, b) ((x) > (b) ? (b) : (x) < (a) ? (a) : (x))
#define STREQ(x, y)    (strcmp(x, y) == 0)

#define ALWAYS_INLINE inline __attribute__((always_inline))

#define M_PI_F ((float) M_PI)

void * malloc_or_die(size_t s);
int open_or_die(const char *pathname, int flags);
FILE * fopen_or_die(const char *path, const char *mode);

ALWAYS_INLINE static uint32_t get_time_ms(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        DIE_HARD("clock_gettime");

    return (uint32_t) (1000 * ts.tv_sec + ts.tv_nsec / 1000000);
}

ALWAYS_INLINE static uint32_t get_time_us(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
        DIE_HARD("clock_gettime");

    return (uint32_t) (1000000 * ts.tv_sec + ts.tv_nsec / 1000);
}

#define SYNCHRONIZED(MUT, CODE) \
    assert(pthread_mutex_lock(&MUT) == 0); \
    CODE \
    assert(pthread_mutex_unlock(&MUT) == 0)

#endif
