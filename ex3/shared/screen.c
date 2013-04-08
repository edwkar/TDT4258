#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "screen.h"
#include "utils.h"

#define IS_TRANSPARENT(v) (v.red == PIXEL_TRANSPARENT.red &&\
                           v.green == PIXEL_TRANSPARENT.green &&\
                           v.blue == PIXEL_TRANSPARENT.blue)

static int fb_file;
static struct pixel *fb;

static struct pixel back_buf[FB_SIZE];

void screen_init(void)
{
    fb_file = open(FRAME_BUFFER_PATH, O_RDWR);
    if (fb_file < 0)
        die_hard("open");

    fb = mmap(NULL, FB_SIZE_BYTES, PROT_READ | PROT_WRITE,
              MAP_SHARED, fb_file, 0);
    if (fb == MAP_FAILED)
        die_hard("mmap");
}

void screen_cleanup(void)
{
    munmap(fb, FB_SIZE_BYTES);
    close(fb_file);
}

void screen_clear(struct pixel color)
{
    screen_draw_rect(0, 0, FB_WIDTH, FB_HEIGHT, color);
}

void screen_put_pixel(uint16_t x, uint16_t y, struct pixel color)
{
    unsigned int idx = (FB_HEIGHT-y)*FB_WIDTH+x;
    if (idx < FB_SIZE)
        back_buf[idx] = color;
}

void screen_draw_rect(unsigned int _x, unsigned int _y,
                      unsigned int w, unsigned int h,
                      struct pixel color)
{
    for (unsigned int y = _y; y < _y+h ; ++y)
        for (unsigned int x = _x; x < _x+w; ++x)
            screen_put_pixel(x, y, color);
}

void screen_draw_sprite(uint16_t _x, uint16_t _y, const struct sprite *s)
{
    for (unsigned int x = _x, s_idx = 0; x < _x+s->width; ++x)
        for (unsigned int y = _y; y < _y+s->height ; ++y, ++s_idx) {
            unsigned int fb_idx = (FB_HEIGHT-y)*FB_WIDTH+x;
            if (fb_idx < FB_SIZE && !IS_TRANSPARENT(s->data[s_idx]))
                back_buf[fb_idx] = s->data[s_idx];
        }
}

void screen_redraw(void)
{
    memcpy(fb, back_buf, sizeof back_buf);
}

