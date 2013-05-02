#ifndef TANK_H
#define TANK_H

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

typedef struct m_Tank {
    GameObject m_base;

    void (*set_input)(struct m_Tank *, TankInput input);

    bool (*has_released)(struct m_Tank *, int32_t *x, int32_t *y,
                         uint32_t *charge, uint32_t *angle);

    bool (*is_updating_from_impact)(const struct m_Tank *);

    void (*clear_release)(struct m_Tank *);

    uint32_t (*get_health)(const struct m_Tank *);

    const Terrain *_terrain;
    TankInput m_input;

    struct sprite *_body_sprite;
    struct sprite *_turret_sprites[TANK_TURRET_MAX_ANG+1];

    bool m_has_released;
    uint32_t m_turret_min_inc;
    uint32_t m_turret_inclination;
    uint32_t m_turret_max_inc;
    uint32_t m_turret_charge;

    int32_t _start_x;

    int32_t m_health;
    int32_t m_health_to_lose;
} Tank;

Tank * Tank_construct(const Terrain *terrain, int32_t start_x, bool is_left);
void Tank_init(Tank *this, const Terrain *terrain, int32_t start_x, 
               bool is_left);

#endif
