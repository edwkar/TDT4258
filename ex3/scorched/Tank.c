#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sprite.h"
#include "screen.h"
#include "utils.h"
#include "Tank.h"

#define TANK_YSHIFT    12
#define TANK_XSHIFT    18
#define TURRET_YSHIFT  -10
#define TURRET_XSHIFT  -20

#define VX_FRAME 2
#define VY_FRAME 2
#define VTURRET_FRAME 2
#define VCHARGE_FRAME 1
#define MIN_XPOS  20
#define MAX_XPOS 300

#define TANK_START_HEALTH   100
#define TURRET_MAX_CHARGE   50

#define HEALTH_BAR_WIDTH   40
#define HEALTH_BAR_HEIGHT   5
#define HEALTH_BAR_YSHIFT -30
#define HEALTH_LOSS_FRAME 2

#define CHARGE_BAR_WIDTH   40
#define CHARGE_BAR_HEIGHT   5
#define CHARGE_BAR_YSHIFT -40


static void Tank_reset(GameObject* thisgo);
static void Tank_update_physics(GameObject* thisgo);
static void Tank_render(GameObject *this);
static void Tank_destruct(GameObject *thisgo);
static void Tank_set_input(Tank *this, TankInput input);
static uint32_t Tank_get_health(const Tank *this);
static bool Tank_has_released(Tank *this,
                              uint16_t *x, uint16_t *y,
                              uint16_t *charge, int16_t *angle);
static bool Tank_is_updating_from_impact(const Tank *this);
static void Tank_clear_release(Tank *this);
static void Tank_apply_impact(GameObject *thisgo, int32_t _x, int32_t _y);

Tank * Tank_construct(const Terrain *terrain, uint16_t start_x, bool is_left) {
    Tank *this = malloc_or_die(sizeof(Tank));
    Tank_init(this, terrain, start_x, is_left);
    return this;
}

void Tank_init(Tank *this, const Terrain *terrain, uint16_t start_x, bool is_left)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->update_physics = Tank_update_physics;
    thisgo->apply_impact = Tank_apply_impact;
    thisgo->reset = Tank_reset;
    thisgo->render = Tank_render;
    thisgo->destruct = Tank_destruct;
    this->set_input = Tank_set_input;
    this->has_released = Tank_has_released;
    this->is_updating_from_impact = Tank_is_updating_from_impact;
    this->get_health = Tank_get_health;
    this->clear_release = Tank_clear_release;
    this->_start_x = start_x;
    this->_terrain = terrain;
    this->_has_released = false;

    if (is_left) {
        this->_turret_min_inc = 0;
        this->_turret_max_inc = 90;
    } else {
        this->_turret_min_inc = 90;
        this->_turret_max_inc = 180;
    }

    thisgo->reset(thisgo);

    this->_body_sprite = sprite_load(SC_RESOURCES_PATH "/tank.image");
    if (!is_left)
        sprite_invert_horizontal(this->_body_sprite);

    for (int i = 0; i <= TANK_TURRET_MAX_ANG; ++i) {
        char fn[256];
        snprintf(fn, sizeof fn, "%s/turret_%d.image",
                 SC_RESOURCES_PATH, i);
        this->_turret_sprites[i] = sprite_load(fn);
    }
}

static void Tank_reset(GameObject* thisgo)
{
    Tank *this = (Tank*) thisgo;

    thisgo->_xpos = this->_start_x;
    thisgo->_ypos = this->_terrain->height_at(this->_terrain, this->_start_x);

    this->_turret_inclination = (this->_turret_max_inc +
                                 this->_turret_min_inc) / 2;
    this->_turret_charge = 0;
    this->_health = TANK_START_HEALTH;
    this->_health_to_lose = 0;
}

static uint32_t Tank_get_health(const Tank *this)
{
    assert(this->_health >= 0);
    return (uint32_t) this->_health;
}

static void Tank_destruct(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;
    sprite_free(this->_body_sprite);
    for (int i = 0; i <= TANK_TURRET_MAX_ANG; ++i)
        sprite_free(this->_turret_sprites[i]);
    free(thisgo);
}

static void Tank_apply_impact(GameObject *thisgo, int32_t x, int32_t y)
{
    Tank *this = (Tank*) thisgo;
    double v = sqrt(  (x-thisgo->_xpos) * (x-thisgo->_xpos)
                    + (y-thisgo->_ypos) * (y-thisgo->_ypos));
    assert(this->_health_to_lose == 0);

    this->_health_to_lose = (int32_t) max(0, max(70-v, 100-pow(v, 1.3)));
    printf("Tank at (%d, %d) loses %d HP (%f)\n", thisgo->_xpos, thisgo->_ypos,
                                                  this->_health_to_lose, v);
}

static void Tank_update_physics(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    if (this->_health_to_lose != 0) {
        this->_health -= min(HEALTH_LOSS_FRAME, this->_health_to_lose);
        this->_health = max(0, this->_health);
        this->_health_to_lose = max(0, this->_health_to_lose-HEALTH_LOSS_FRAME);
    }

    if (this->_input.move_left)
        thisgo->_xpos -= VX_FRAME;
    if (this->_input.move_right)
        thisgo->_xpos += VX_FRAME;

    if (this->_input.turret_left)
        this->_turret_inclination -= VTURRET_FRAME;
    if (this->_input.turret_right)
        this->_turret_inclination += VTURRET_FRAME;

    thisgo->_xpos = clamp(MIN_XPOS, thisgo->_xpos, MAX_XPOS);

    uint16_t ter_height_at = this->_terrain->height_at(this->_terrain,
                                                       thisgo->_xpos);
    thisgo->_ypos = max(ter_height_at + TANK_YSHIFT,
                        thisgo->_ypos - VY_FRAME);

    this->_turret_inclination = clamp(this->_turret_min_inc,
                                      this->_turret_inclination,
                                      this->_turret_max_inc);

    this->_has_released = this->_turret_charge != 0 && !this->_input.turret_charge;
    if (this->_input.turret_charge)
        this->_turret_charge = min(this->_turret_charge + VCHARGE_FRAME,
                                   TURRET_MAX_CHARGE);

    this->_input = (TankInput){ false, false, false, false, false };
}

static bool Tank_has_released(Tank *this,
                              uint16_t *x, uint16_t *y,
                              uint16_t *charge, int16_t *angle) {
    GameObject *thisgo = (GameObject*) this;

    if (!this->_has_released)
        return false;
    else {
        *x = thisgo->_xpos;
        *y = thisgo->_ypos;
        *charge = this->_turret_charge;
        *angle = this->_turret_inclination;
        return true;
    }
}

static bool Tank_is_updating_from_impact(const Tank *this) {
    return this->_health_to_lose != 0;
}

static void Tank_clear_release(Tank *this) {
    this->_turret_charge = 0;
    this->_has_released = false;

}

static void Tank_render(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    screen_draw_sprite(thisgo->_xpos+TURRET_XSHIFT,
                       thisgo->_ypos+TURRET_YSHIFT,
                       this->_turret_sprites[this->_turret_inclination]);
    screen_draw_sprite(thisgo->_xpos-TANK_XSHIFT, thisgo->_ypos-TANK_YSHIFT,
                       this->_body_sprite);
    screen_draw_rect(thisgo->_xpos-1, thisgo->_ypos-1, 3, 3, PIXEL_RED);

    screen_draw_rect(thisgo->_xpos-HEALTH_BAR_WIDTH/2,
                     thisgo->_ypos+HEALTH_BAR_YSHIFT,
                     HEALTH_BAR_WIDTH,
                     HEALTH_BAR_HEIGHT,
                     PIXEL_RED);
    screen_draw_rect(thisgo->_xpos-HEALTH_BAR_WIDTH/2,
                     thisgo->_ypos+HEALTH_BAR_YSHIFT,
                     HEALTH_BAR_WIDTH * this->_health / TANK_START_HEALTH,
                     HEALTH_BAR_HEIGHT,
                     PIXEL_GREEN);

    if (this->_turret_charge) {
        screen_draw_rect(thisgo->_xpos-CHARGE_BAR_WIDTH/2,
                         thisgo->_ypos+CHARGE_BAR_YSHIFT,
                         CHARGE_BAR_WIDTH,
                         CHARGE_BAR_HEIGHT,
                         PIXEL_GREY);
        screen_draw_rect(thisgo->_xpos-CHARGE_BAR_WIDTH/2,
                         thisgo->_ypos+CHARGE_BAR_YSHIFT,
                         CHARGE_BAR_WIDTH * this->_turret_charge /
                                            TURRET_MAX_CHARGE,
                         CHARGE_BAR_HEIGHT,
                         PIXEL_WHITE);
    }
}

static void Tank_set_input(Tank * this, TankInput input)
{
    this->_input = input;
}
