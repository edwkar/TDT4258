#ifndef __TANK_H
#define __TANK_H

#include <stdbool.h>
#include "sprite.h"
#include "GameObject.h"
#include "Terrain.h"

#define TANK_TURRET_MAX_ANG 180

typedef struct {
    bool move_left; 
    bool move_right; 
    bool turret_left;
    bool turret_right;
    bool turret_charge;
} TankInput;

typedef struct _Tank {
    GameObject _base;
    struct sprite *_body_sprite;
    struct sprite *_turret_sprites[TANK_TURRET_MAX_ANG+1];
    const Terrain *_terrain;
    TankInput _input;
    bool _has_released;
    int16_t _turret_min_inc;
    int16_t _turret_inclination;
    int16_t _turret_max_inc;
    uint16_t _turret_charge;
    int16_t _start_x;
    int16_t _health;
    int16_t _health_to_lose;

    void (*set_input)(struct _Tank *, TankInput input);
    bool (*has_released)(struct _Tank *, uint16_t *x, uint16_t *y,
                         uint16_t *charge, int16_t *angle);
    bool (*is_updating_from_impact)(const struct _Tank *);
    void (*clear_release)(struct _Tank *);
    uint32_t (*get_health)(const struct _Tank *);
} Tank;

Tank * Tank_construct(const Terrain *terrain, uint16_t start_x, bool is_left);
void Tank_init(Tank *this, const Terrain *terrain, uint16_t start_x, bool is_left);

#endif
