#include <assert.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "profiling.h"
#include "screen.h"
#include "utils.h"


static int fb_fd = -1;
static struct pixel *fb = NULL;

static struct pixel back_buf[FB_SIZE];

static bool screen_is_shaking = false;
static uint8_t screen_opacity = SCREEN_MAX_OPACITY;

void screen_init(void)
{
    assert(fb_fd == -1);

    fb_fd = open_or_die(FRAME_BUFFER_PATH, O_RDWR);

    fb = mmap(NULL, FB_SIZE_BYTES, PROT_READ | PROT_WRITE,
              MAP_SHARED, fb_fd, 0);

    if (fb == MAP_FAILED)
        DIE_HARD("map");
}

void screen_set_shaking(bool new_val)
{
    screen_is_shaking = new_val;
}

void screen_set_opacity(uint8_t val)
{
    screen_opacity = val;
}

void screen_increment_opacity(uint8_t delta)
{
    screen_opacity = (uint8_t) MIN(SCREEN_MAX_OPACITY,
                                   ((uint32_t) screen_opacity)+delta);
}

void screen_cleanup(void)
{
    munmap(fb, FB_SIZE_BYTES);
    fb = NULL;
    close(fb_fd);
}

void screen_clear(struct pixel color)
{
    screen_draw_rect(0, 0, FB_WIDTH, FB_HEIGHT, color);
}

inline static void screen_put_pixel(uint32_t x, uint32_t y,
                                    struct pixel color)
{
    unsigned int idx = (FB_HEIGHT-y)*FB_WIDTH+x;
    if (idx < FB_SIZE)
        back_buf[idx] = color;
}

PROFILING_SETUP(screen_draw_rect);
void screen_draw_rect(int32_t from_x, int32_t from_y,
                      unsigned int w, unsigned int h,
                      struct pixel color)
{
    PROFILING_ENTER(screen_draw_rect);

    for (int32_t i = MAX(-from_x, 0); i < (int32_t) w; ++i)
        for (int32_t j = MAX(-from_y, 0); j < (int32_t) h; ++j)
            screen_put_pixel((uint32_t) (from_x+i), (uint32_t) (from_y+j), color);

    PROFILING_EXIT(screen_draw_rect);
}

PROFILING_SETUP(screen_draw_sprite_raw);
void screen_draw_sprite_raw(struct sprite *s)
{
    PROFILING_ENTER(screen_draw_sprite_raw);

    assert(s->m_width == FB_WIDTH && s->m_height == FB_HEIGHT);

    for (uint32_t y = 0; y < s->m_height; ++y) {
        uint32_t sprite_idx = y * FB_WIDTH;
        uint32_t fb_idx = (FB_HEIGHT-1-y) * FB_WIDTH;

        memcpy(back_buf + fb_idx,
               s->_data + sprite_idx,
               FB_WIDTH * sizeof(struct pixel));
    }

    PROFILING_EXIT(screen_draw_sprite_raw);
}

PROFILING_SETUP(screen_draw_sprite);
void screen_draw_sprite(int32_t from_x, int32_t from_y, struct sprite *s)
{
    PROFILING_ENTER(screen_draw_sprite);

    int32_t y = from_y + (int32_t) s->m_y_draw_start;
    if (y < 0)
        y = 0;

    int32_t tom_y = from_y + (int32_t) s->m_y_draw_end;
    if (y > FB_HEIGHT)
        y = FB_HEIGHT;

    for ( ; y < tom_y; ++y) {
        int32_t x    = from_x;
        int32_t tom_x = MIN(FB_WIDTH, from_x + (int32_t) s->m_width);

        /* Remember, we must draw "upside down".
         */
        int32_t fb_idx = (FB_HEIGHT-1-y) * FB_WIDTH + x;

        int32_t sprite_idx = (y - from_y) * (int32_t) s->m_width;

        while (x < 0)
            ++x, ++fb_idx, ++sprite_idx;

        struct pixel * restrict bb = &back_buf[fb_idx];
        struct pixel * restrict ss = &s->_data[sprite_idx];

        for ( ; x < tom_x; ++x, ++bb, ++ss)
            if (!IS_TRANSPARENT(ss))
                *bb = *ss;
    }

    PROFILING_EXIT(screen_draw_sprite);
}

ALWAYS_INLINE static void apply_opacity(uint8_t *v) {
    *v = (*v * screen_opacity) / SCREEN_MAX_OPACITY;
}

static void screen_apply_opacity(void)
{
    for (size_t i = 0; i < FB_SIZE; ++i) {
        apply_opacity(&back_buf[i].red);
        apply_opacity(&back_buf[i].green);
        apply_opacity(&back_buf[i].blue);
    }
}

PROFILING_SETUP(screen_redraw);
void screen_redraw(void)
{
    PROFILING_ENTER(screen_redraw);

    if (screen_opacity != SCREEN_MAX_OPACITY)
        screen_apply_opacity();

    bool should_shake = screen_is_shaking && rand()%2 == 0;

    if (should_shake) {
        size_t shift = 1 + rand()%5;
        void *pos = fb + shift * FB_WIDTH;
        size_t n = sizeof back_buf - shift * FB_WIDTH * sizeof(struct pixel);
        memcpy(pos, back_buf, n);
    } else
        memcpy(fb, back_buf, sizeof back_buf);

    PROFILING_EXIT(screen_redraw);
}
