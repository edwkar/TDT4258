#ifndef __TERRAIN_H
#define __TERRAIN_H

#include <stdint.h>

#include "sprite.h"

#include "GameObject.h"


typedef struct _Terrain {
    GameObject _base;

    int32_t (*height_at)(const struct _Terrain *this, int32_t xpos);

    uint32_t _width;
    uint32_t _height;
    uint8_t* _grid;

    struct sprite *_bg_sprite;
    struct sprite *_sprite;
} Terrain;

Terrain * Terrain_construct(uint32_t width, uint32_t height);
void Terrain_init(Terrain *this, uint32_t width, uint32_t height);

#endif
