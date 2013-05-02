#include <stdio.h>
#include <stdlib.h>
#include "synthesizer.h"
#include "utils.h"

static const struct melody MELODIES[] = {
    #include "melodiesrendered.c"
};

const struct melody *melodies_get(const char *name)
{
    int num_melodies = sizeof MELODIES / sizeof MELODIES[0];

    for (int i = 0; i < num_melodies; ++i)
        if (streq(name, MELODIES[i].name))
            return &MELODIES[i];

    PANIC("Unknown melody!");
}
