#ifndef __PROJECTILE_H
#define __PROJECTILE_H

#include <stdbool.h>
#include <stdint.h>
#include "sprite.h"
#include "GameObject.h"
#include "Terrain.h"

typedef struct _Projectile {
    GameObject _base;
    struct sprite *_sprite;
    void (*fire_from)(struct _Projectile *this, uint16_t x, uint16_t y,
                      uint16_t angle, double power);
    void (*reset)(struct _Projectile *this);
    bool (*has_landed)(const struct _Projectile *this, 
                       uint16_t landing_pos[static 2]);
    Terrain *_terrain;
    float _x;
    float _y;
    float _vx;
    float _vy;
    bool _is_active;
} Projectile;

Projectile * Projectile_construct(Terrain *terrain);
void Projectile_init(Projectile *this, Terrain *terrain);

#endif
