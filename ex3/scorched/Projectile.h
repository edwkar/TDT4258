#ifndef __PROJECTILE_H
#define __PROJECTILE_H

#include <stdbool.h>
#include <stdint.h>

#include "sprite.h"

#include "GameObject.h"
#include "Terrain.h"


typedef struct _Projectile {
    GameObject _base;

    void (*reset)(struct _Projectile *this);

    void (*fire_from)(struct _Projectile *this, int32_t x, int32_t y,
                      uint32_t angle, float power);

    bool (*has_landed)(const struct _Projectile *this, 
                       int32_t landing_pos[static 2]);

    Terrain *_terrain;

    bool _is_active;

    float _x;
    float _y;
    float _vx;
    float _vy;

    struct sprite *_sprite;
} Projectile;


Projectile * Projectile_construct(Terrain *terrain);
void Projectile_init(Projectile *this, Terrain *terrain);

#endif
