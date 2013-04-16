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

#define TANK_INPUT_ALL_OFF ((TankInput) { false, false, false,   \
                                          false, false         })

typedef struct _Tank {
    GameObject _base;

    void (*set_input)(struct _Tank *, TankInput input);

    bool (*has_released)(struct _Tank *, int32_t *x, int32_t *y,
                         uint32_t *charge, uint32_t *angle);

    bool (*is_updating_from_impact)(const struct _Tank *);

    void (*clear_release)(struct _Tank *);

    uint32_t (*get_health)(const struct _Tank *);

    const Terrain *_terrain;
    TankInput _input;

    struct sprite *_body_sprite;
    struct sprite *_turret_sprites[TANK_TURRET_MAX_ANG+1];

    bool _has_released;
    uint32_t _turret_MIN_inc;
    uint32_t _turret_inclination;
    uint32_t _turret_MAX_inc;
    uint32_t _turret_charge;

    int32_t _start_x;

    int32_t _health;
    int32_t _health_to_lose;
} Tank;

Tank * Tank_construct(const Terrain *terrain, int32_t start_x, bool is_left);
void Tank_init(Tank *this, const Terrain *terrain, int32_t start_x, 
               bool is_left);

#endif
