#ifndef SPRITE_H
#define SPRITE_H

#include <stdbool.h>
#include <stdint.h>


struct pixel;

struct sprite {
    uint32_t m_width;
    uint32_t m_height;
    struct pixel *_data;

    uint32_t m_y_draw_start;
    uint32_t m_y_draw_end;
};

struct sprite * sprite_construct(uint32_t width, uint32_t height);
struct sprite * sprite_load(const char *path);
void sprite_invert_horizontal(struct sprite *s);
void sprite_free(struct sprite *s);

#define sprite_get_pixel(s, x, y)    (s->_data[(y)*s->m_width+(x)])
#define sprite_set_pixel(s, x, y, p) (s->_data[(y)*s->m_width+(x)] = (p))

#endif
