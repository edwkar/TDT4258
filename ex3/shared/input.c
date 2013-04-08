#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "input.h"
#include "utils.h"

static int input_fd = -1;

void input_init(void) {
    if ((input_fd = open(INPUT_DRIVER_PATH, O_RDONLY)) < 0)
        die_hard("open");
}

bool input_key_is_down(enum input_key key) {
    assert(input_fd != -1);

#ifdef HOST_BUILD
    if (lseek(input_fd, 0, SEEK_SET) != 0)
        die_hard("lseek");
#endif

    unsigned char v = 0;
    if (read(input_fd, &v, sizeof v) != sizeof v)
        die_hard("read");

    return v & key;
}
