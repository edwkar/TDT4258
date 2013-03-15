#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "leds.h"
#include "utils.h"

bool streq(const char a[static 1], const char b[static 1])
{
    return strcmp(a, b) == 0;
}

/* For debug use, and tests.
*/
static volatile int a, b;
void tiny_spinlock_wait(void)
{
    asm ("");
    for (a = 600; a--; )
        for (b = 600; b--; )
            ;
}
