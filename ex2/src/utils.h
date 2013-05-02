#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define ALWAYS_INLINE(fn) inline fn attribute((always_inline)) 
#define MEMORY_BARRIER asm volatile ("" : : : "memory")

#define PANIC(msg) do {\
            fprintf(stderr, "%s", msg);\
            assert(streq(msg, "foo"));\
            exit(EXIT_FAILURE);\
        } while (true);

int max(int x, int y);
bool streq(const char a[static 1], const char b[static 1]);
void tiny_spinlock_wait(void);

#endif
