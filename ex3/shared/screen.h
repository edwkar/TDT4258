#ifndef __SCREEN__H
#define __SCREEN__H

#include <stdint.h>
#include "sprite.h"

#define FB_WIDTH         320
#define FB_HEIGHT        240
#define FB_SIZE          FB_WIDTH * FB_HEIGHT
#define FB_SIZE_BYTES    FB_SIZE * 3

struct pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};

#define PIXEL_BLACK       ((struct pixel){   0,   0,   0 })
#define PIXEL_RED         ((struct pixel){   0,   0, 255 })
#define PIXEL_GREEN       ((struct pixel){   0, 255,   0 })
#define PIXEL_BROWN       ((struct pixel){   0,   0, 150 })
#define PIXEL_TRANSPARENT ((struct pixel){   1,   2,   3 })
#define PIXEL_GREY        ((struct pixel){  90,  90,  90 })
#define PIXEL_WHITE       ((struct pixel){ 255, 255, 255 })
#define PIXEL_TRANSPARENT ((struct pixel){   1,   2,   3 })

void screen_init(void);
void screen_clear(struct pixel color);
void screen_draw_rect(unsigned int x, unsigned int y,
                      unsigned int w, unsigned int h,
                      struct pixel color);
void screen_draw_sprite(uint16_t _x, uint16_t _y, const struct sprite *s);
// XXX ALWAYS INLINE
void screen_put_pixel(uint16_t _x, uint16_t _y, struct pixel color);
void screen_redraw(void);
void screen_cleanup(void);

#endif
