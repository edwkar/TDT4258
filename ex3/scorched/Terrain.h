#ifndef __TERRAIN_H
#define __TERRAIN_H

#include <stdint.h>
#include "sprite.h"
#include "GameObject.h"

typedef struct _Terrain {
    GameObject _base;
    uint16_t _width;
    uint16_t _height;
    uint8_t* _grid;
    struct sprite *_sprite;
    uint16_t (*height_at)(const struct _Terrain *this, uint16_t xpos);
} Terrain;

Terrain * Terrain_construct(uint16_t width, uint16_t height);
void Terrain_init(Terrain *this, uint16_t width, uint16_t height);

#endif
