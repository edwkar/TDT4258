#ifndef __SPRITE_H
#define __SPRITE_H

#include <stdint.h>

struct pixel;

struct sprite {
    uint16_t width;
    uint16_t height;
    struct pixel *data;
};

struct sprite * sprite_construct(uint16_t width, uint16_t height);
struct sprite * sprite_load(const char *path);
void sprite_invert_horizontal(struct sprite *s);
void sprite_free(struct sprite *s);

#endif
