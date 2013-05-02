#ifndef INPUTH
#define INPUTH

#include <stdbool.h>

enum input_key {
    input_0 = 1U << 0,
    input_1 = 1U << 1,
    input_2 = 1U << 2,
    input_3 = 1U << 3,
    input_4 = 1U << 4,
    input_5 = 1U << 5,
    input_6 = 1U << 6,
    input_7 = 1U << 7
};

void input_init(void);
bool input_key_is_down(enum input_key key);
void input_cleanup(void);

#endif
