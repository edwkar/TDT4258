#ifndef TERRAIN_H
#define TERRAIN_H

#include <stdint.h>

#include "sprite.h"

#include "GameObject.h"


typedef struct m_Terrain {
    GameObject m_base;

    int32_t (*height_at)(const struct m_Terrain *this, int32_t xpos);

    uint32_t m_width;
    uint32_t m_height;
    uint8_t* m_grid;

    struct sprite *_bg_sprite;
    struct sprite *_sprite;
} Terrain;

Terrain * Terrain_construct(uint32_t width, uint32_t height);
void Terrain_init(Terrain *this, uint32_t width, uint32_t height);

#endif
