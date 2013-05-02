#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "leds.h"
#include "utils.h"


static int leds_fd = -1;

void leds_init(void)
{
    assert(leds_fd == -1);
    leds_fd = open_or_die(LEDS_DEVICE_PATH, O_WRONLY);
}

void leds_set(uint8_t config)
{
    assert(leds_fd != -1);

    unsigned char v[1];
    v[0] = (unsigned char) config;

    if (write(leds_fd, &v, sizeof v) != sizeof v)
        DIE_HARD("write");
}

void leds_cleanup(void)
{
    if (close(leds_fd) != 0)
        DIE_HARD("leds");
}
