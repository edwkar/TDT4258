#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"
#include "sprite.h"
#include "utils.h"

#include "Tank.h"


#define MIN_XPOS             20
#define MAX_XPOS            300
#define VCHARGE_FRAME         1

#define TURRET_YSHIFT       -10
#define TURRET_XSHIFT       -20
#define VTURRET_FRAME         2

#define VX_FRAME              2
#define VY_FRAME              2

#define TANK_START_HEALTH   100
#define HEALTH_LOSS_FRAME     2

#define TURRET_MAX_CHARGE    50

#define BODY_YSHIFT          12
#define BODY_XSHIFT          18

#define BAR_WIDTH            40
#define BAR_HEIGHT            5

#define HEALTH_BAR_YSHIFT   -30
#define CHARGE_BAR_YSHIFT   -40

static void Tank_reset(GameObject *);
static void Tank_update(GameObject *);
static void Tank_render(GameObject *);
static void Tank_destruct(GameObject *);
static void Tank_set_input(Tank *, TankInput);
static uint32_t Tank_get_health(const Tank *);
static bool Tank_has_released(Tank *,
                              int32_t *, int32_t *,
                              uint32_t *, uint32_t *);
static bool Tank_is_updating_from_impact(const Tank *);
static void Tank_clear_release(Tank *);
static void Tank_apply_impact(GameObject *, int32_t, int32_t);

Tank * Tank_construct(const Terrain *terrain, int32_t start_x, bool is_left)
{
    Tank *this = malloc_or_die(sizeof(Tank));
    Tank_init(this, terrain, start_x, is_left);
    return this;
}

void Tank_init(Tank *this, const Terrain *terrain, int32_t start_x,
               bool is_left)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->update = Tank_update;
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
    this->_input = TANK_INPUT_ALL_OFF;

    if (is_left) {
        this->_turret_MIN_inc = 0;
        this->_turret_MAX_inc = 90;
    } else {
        this->_turret_MIN_inc = 90;
        this->_turret_MAX_inc = 180;
    }

    thisgo->reset(thisgo);

    /* Load sprites.
     */
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

    this->_turret_inclination = (this->_turret_MAX_inc +
                                 this->_turret_MIN_inc) / 2;
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

    /* Free sprites.
     */
    sprite_free(this->_body_sprite);
    for (int i = 0; i <= TANK_TURRET_MAX_ANG; ++i)
        sprite_free(this->_turret_sprites[i]);

    free(thisgo);
}

static void Tank_set_input(Tank *this, TankInput input)
{
    this->_input = input;
}

static void Tank_apply_impact(GameObject *thisgo, int32_t x, int32_t y)
{
    Tank *this = (Tank*) thisgo;
    assert(this->_health_to_lose == 0);

    double dist = sqrt(  (x-thisgo->_xpos) * (x-thisgo->_xpos)
                       + (y-thisgo->_ypos) * (y-thisgo->_ypos));

    /* Found through careful messing around at Wolfram Alpha.
     */
    this->_health_to_lose = (int32_t) MAX(0,
                                          MAX(100 - pow(dist, 2.8), 40  - dist));
}

static void Tank_update(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    if (this->_health_to_lose != 0) {
        this->_health -= MIN(HEALTH_LOSS_FRAME, this->_health_to_lose);
        this->_health = MAX(0, this->_health);

        this->_health_to_lose = MAX(0,
                                    this->_health_to_lose - HEALTH_LOSS_FRAME);
    }

    if (this->_input.move_left)
        thisgo->_xpos -= VX_FRAME;
    if (this->_input.move_right)
        thisgo->_xpos += VX_FRAME;

    if (this->_input.turret_left)
        this->_turret_inclination -= VTURRET_FRAME;
    if (this->_input.turret_right)
        this->_turret_inclination += VTURRET_FRAME;

    thisgo->_xpos = CLAMP(MIN_XPOS, thisgo->_xpos, MAX_XPOS);

    int32_t ter_height_at = this->_terrain->height_at(this->_terrain,
                                                       thisgo->_xpos);
    thisgo->_ypos = MAX(ter_height_at + BODY_YSHIFT,
                        thisgo->_ypos - VY_FRAME);

    this->_turret_inclination = CLAMP(this->_turret_MIN_inc,
                                      this->_turret_inclination,
                                      this->_turret_MAX_inc);

    this->_has_released = this->_turret_charge != 0 && !this->_input.turret_charge;
    if (this->_input.turret_charge)
        this->_turret_charge = MIN(this->_turret_charge + VCHARGE_FRAME,
                                   TURRET_MAX_CHARGE);

    this->_input = TANK_INPUT_ALL_OFF;
}

static bool Tank_has_released(Tank *this,
                              int32_t *x, int32_t *y,
                              uint32_t *charge, uint32_t *angle)
{
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

static bool Tank_is_updating_from_impact(const Tank *this)
{
    return this->_health_to_lose != 0;
}

static void Tank_clear_release(Tank *this)
{
    this->_turret_charge = 0;
    this->_has_released = false;
}


/* RENDERING
 */
static void Tank_render_body_and_turret(GameObject *thisgo);
static void Tank_render_health_indicator(GameObject *thisgo);
static void Tank_render_charge_indicator(GameObject *thisgo);

static void Tank_render(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    Tank_render_body_and_turret(thisgo);
    Tank_render_health_indicator(thisgo);

    if (this->_turret_charge)
        Tank_render_charge_indicator(thisgo);
}

static void Tank_render_body_and_turret(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    int32_t shake_dx = 0,
            shake_dy = 0;
    if (Tank_is_updating_from_impact(this)) {
        shake_dx = rand()%4;
        shake_dy = rand()%4;
    }

    screen_draw_sprite(thisgo->_xpos+TURRET_XSHIFT + shake_dx,
                       thisgo->_ypos+TURRET_YSHIFT + shake_dy,
                       this->_turret_sprites[this->_turret_inclination]);

    screen_draw_sprite(thisgo->_xpos-BODY_XSHIFT + shake_dx,
                       thisgo->_ypos-BODY_YSHIFT + shake_dy,
                       this->_body_sprite);

#ifndef NDEBUG
    screen_draw_rect(thisgo->_xpos - 1, thisgo->_ypos - 1,
                     3, 3, PIXEL_RED);

    screen_draw_rect(thisgo->_xpos+TURRET_XSHIFT + shake_dx - 1,
                     thisgo->_ypos+TURRET_YSHIFT + shake_dy - 1,
                     3, 3, PIXEL_GREEN);


    screen_draw_rect(thisgo->_xpos-BODY_XSHIFT + shake_dx - 1,
                     thisgo->_ypos-BODY_YSHIFT + shake_dy - 1,
                     3, 3, PIXEL_BLUE);
#endif
}

static void Tank_render_indicator(GameObject *,
                                  struct pixel,
                                  struct pixel,
                                  int32_t,
                                  uint32_t);

static void Tank_render_health_indicator(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    uint32_t width =
        (uint32_t) ((BAR_WIDTH * this->_health) / TANK_START_HEALTH);

    if (this->_health > 0 && width == 0)
        width = 1;

    Tank_render_indicator(thisgo,
                          PIXEL_RED, PIXEL_GREEN,
                          HEALTH_BAR_YSHIFT, width);
}

static void Tank_render_charge_indicator(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    uint32_t width =
        (uint32_t) ((BAR_WIDTH * this->_turret_charge) / TURRET_MAX_CHARGE);

    Tank_render_indicator(thisgo,
                          PIXEL_GREY, PIXEL_WHITE,
                          CHARGE_BAR_YSHIFT, width);
}

static void Tank_render_indicator(GameObject *thisgo,
                                  struct pixel back_color,
                                  struct pixel front_color,
                                  int32_t y_shift,
                                  uint32_t front_width)
{
    int32_t x = thisgo->_xpos - BAR_WIDTH/2;
    int32_t y = MAX(0, thisgo->_ypos + y_shift);

    screen_draw_rect(x, y,
                     BAR_WIDTH, BAR_HEIGHT,
                     back_color);

    screen_draw_rect(x, y,
                     front_width, BAR_HEIGHT,
                     front_color);
}
