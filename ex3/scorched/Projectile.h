#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdbool.h>
#include <stdint.h>

#include "sprite.h"

#include "GameObject.h"
#include "Terrain.h"


typedef struct m_Projectile {
    GameObject m_base;

    void (*reset)(struct m_Projectile *this);

    void (*fire_from)(struct m_Projectile *this, int32_t x, int32_t y,
                      uint32_t angle, float power);

    bool (*has_landed)(const struct m_Projectile *this, 
                       int32_t landing_pos[static 2]);

    Terrain *_terrain;

    bool m_is_active;

    float m_x;
    float m_y;
    float m_vx;
    float m_vy;

    struct sprite *_sprite;
} Projectile;


Projectile * Projectile_construct(Terrain *terrain);
void Projectile_init(Projectile *this, Terrain *terrain);

#endif
