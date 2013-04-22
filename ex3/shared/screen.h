#ifndef SCREENH
#define SCREENH

#include <stdbool.h>
#include <stdint.h>

#include "sprite.h"


#define SCREEN_MAX_OPACITY 255U

#define FB_WIDTH           320
#define FB_HEIGHT          240
#define FB_SIZE            FB_WIDTH * FB_HEIGHT
#define FB_SIZE_BYTES      FB_SIZE * 3


struct pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} __attribute__((packed));

#define PIXEL(r, g, b)  ((struct pixel){ .red=(r), .green=(g), .blue=(b) })

#define PIXEL_BLACK       PIXEL(  0,   0,   0)
#define PIXEL_BLUE        PIXEL(  0,   0, 255)
#define PIXEL_RED         PIXEL(255,   0,   0)
#define PIXEL_GREEN       PIXEL(  0, 255,   0)
#define PIXEL_BROWN       PIXEL( 40,  40,  40)
#define PIXEL_GREY        PIXEL( 90,  90,  90)
#define PIXEL_WHITE       PIXEL(255, 255, 255)
#define PIXEL_TRANSPARENT PIXEL(  1,   1,   1)

#define IS_TRANSPARENT(p) ((p)->red == 1 && (p)->green == 1 && (p)->blue == 1)


void screen_init(void);
void screen_clear(struct pixel color);
void screen_draw_rect(int32_t x, int32_t y,
                      uint32_t w, uint32_t h,
                      struct pixel color);
void screen_draw_sprite(int32_t m_x, int32_t m_y, struct sprite *s);
void screen_draw_sprite_raw(struct sprite *s);
void screen_set_shaking(bool new_val);
void screen_set_opacity(uint8_t val);
void screen_increment_opacity(uint8_t delta);
void screen_redraw(void);
void screen_cleanup(void);

#endif
