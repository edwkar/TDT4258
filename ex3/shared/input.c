#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "input.h"
#include "utils.h"


static int input_fd = -1;

void input_init(void)
{
    assert(input_fd == -1);
    input_fd = open_or_die(INPUT_DEVICE_PATH, O_RDONLY);
}

bool input_key_is_down(enum input_key key)
{
    assert(input_fd != -1);

#ifdef HOST_BUILD
    if (lseek(input_fd, 0, SEEK_SET) != 0)
        DIE_HARD("lseek");
#endif

    unsigned char v[1];
    if (read(input_fd, v, sizeof v) != sizeof v)
        DIE_HARD("read");

    return v[0] & key;
}

void input_cleanup(void)
{
    if (close(input_fd) != 0)
        DIE_HARD("close");
}
