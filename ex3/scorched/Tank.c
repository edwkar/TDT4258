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
static bool Tankm_has_released(Tank *,
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
    this->has_released = Tankm_has_released;
    this->is_updating_from_impact = Tank_is_updating_from_impact;
    this->get_health = Tank_get_health;
    this->clear_release = Tank_clear_release;

    this->_start_x = start_x;
    this->_terrain = terrain;
    this->m_has_released = false;
    this->m_input = TANK_INPUT_ALL_OFF;

    if (is_left) {
        this->m_turret_MIN_inc = 0;
        this->m_turret_MAX_inc = 90;
    } else {
        this->m_turret_MIN_inc = 90;
        this->m_turret_MAX_inc = 180;
    }

    thisgo->reset(thisgo);

    /* Load sprites.
     */
    this->_body_sprite = sprite_load(SC_RESOURCES_PATH "/sprites/tank.image");
    if (!is_left)
        sprite_invert_horizontal(this->_body_sprite);

    for (int i = 0; i <= TANK_TURRET_MAX_ANG; ++i) {
        char fn[256];
        snprintf(fn, sizeof fn, "%s/sprites/turret_%d.image",
                 SC_RESOURCES_PATH, i);
        this->_turret_sprites[i] = sprite_load(fn);
    }
}

static void Tank_reset(GameObject* thisgo)
{
    Tank *this = (Tank*) thisgo;

    thisgo->m_xpos = this->_start_x;
    thisgo->m_ypos = this->_terrain->height_at(this->_terrain, this->_start_x);

    this->m_turret_inclination = (this->m_turret_MAX_inc +
                                 this->m_turret_MIN_inc) / 2;
    this->m_turret_charge = 0;
    this->m_health = TANK_START_HEALTH;
    this->m_health_to_lose = 0;
}

static uint32_t Tank_get_health(const Tank *this)
{
    assert(this->m_health >= 0);
    return (uint32_t) this->m_health;
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
    this->m_input = input;
}

static void Tank_apply_impact(GameObject *thisgo, int32_t x, int32_t y)
{
    Tank *this = (Tank*) thisgo;
    assert(this->m_health_to_lose == 0);

    double dist = sqrt(  (x-thisgo->m_xpos) * (x-thisgo->m_xpos)
                       + (y-thisgo->m_ypos) * (y-thisgo->m_ypos));

    /* Found through careful messing around at Wolfram Alpha.
     */
    this->m_health_to_lose = (int32_t) MAX(0,
                                          MAX(100 - pow(dist, 1), 40  - dist));
                                          //MAX(100 - pow(dist, 2.8), 40  - dist));
}

static void Tank_update(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    if (this->m_health_to_lose != 0) {
        this->m_health -= MIN(HEALTH_LOSS_FRAME, this->m_health_to_lose);
        this->m_health = MAX(0, this->m_health);

        this->m_health_to_lose = MAX(0,
                                    this->m_health_to_lose - HEALTH_LOSS_FRAME);
    }

    if (this->m_input.move_left)
        thisgo->m_xpos -= VX_FRAME;
    if (this->m_input.move_right)
        thisgo->m_xpos += VX_FRAME;

    if (this->m_input.turret_left)
        this->m_turret_inclination -= VTURRET_FRAME;
    if (this->m_input.turret_right)
        this->m_turret_inclination += VTURRET_FRAME;

    thisgo->m_xpos = CLAMP(MIN_XPOS, thisgo->m_xpos, MAX_XPOS);

    int32_t term_height_at = this->_terrain->height_at(this->_terrain,
                                                       thisgo->m_xpos);
    thisgo->m_ypos = MAX(term_height_at + BODY_YSHIFT,
                        thisgo->m_ypos - VY_FRAME);

    this->m_turret_inclination = CLAMP(this->m_turret_MIN_inc,
                                      this->m_turret_inclination,
                                      this->m_turret_MAX_inc);

    this->m_has_released = this->m_turret_charge != 0 && !this->m_input.turret_charge;
    if (this->m_input.turret_charge)
        this->m_turret_charge = MIN(this->m_turret_charge + VCHARGE_FRAME,
                                   TURRET_MAX_CHARGE);

    this->m_input = TANK_INPUT_ALL_OFF;
}

static bool Tankm_has_released(Tank *this,
                              int32_t *x, int32_t *y,
                              uint32_t *charge, uint32_t *angle)
{
    GameObject *thisgo = (GameObject*) this;

    if (!this->m_has_released)
        return false;
    else {
        *x = thisgo->m_xpos;
        *y = thisgo->m_ypos;
        *charge = this->m_turret_charge;
        *angle = this->m_turret_inclination;
        return true;
    }
}

static bool Tank_is_updating_from_impact(const Tank *this)
{
    return this->m_health_to_lose != 0;
}

static void Tank_clear_release(Tank *this)
{
    this->m_turret_charge = 0;
    this->m_has_released = false;
}


/* RENDERING
 */
static void Tank_render_body_and_turret(GameObject *thisgo);
static void Tank_renderm_health_indicator(GameObject *thisgo);
static void Tank_render_charge_indicator(GameObject *thisgo);

static void Tank_render(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    Tank_render_body_and_turret(thisgo);
    Tank_renderm_health_indicator(thisgo);

    if (this->m_turret_charge)
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

    screen_draw_sprite(thisgo->m_xpos+TURRET_XSHIFT + shake_dx,
                       thisgo->m_ypos+TURRET_YSHIFT + shake_dy,
                       this->_turret_sprites[this->m_turret_inclination]);

    screen_draw_sprite(thisgo->m_xpos-BODY_XSHIFT + shake_dx,
                       thisgo->m_ypos-BODY_YSHIFT + shake_dy,
                       this->_body_sprite);

#ifndef NDEBUG
    screen_draw_rect(thisgo->m_xpos - 1, thisgo->m_ypos - 1,
                     3, 3, PIXEL_RED);

    screen_draw_rect(thisgo->m_xpos+TURRET_XSHIFT + shake_dx - 1,
                     thisgo->m_ypos+TURRET_YSHIFT + shake_dy - 1,
                     3, 3, PIXEL_GREEN);


    screen_draw_rect(thisgo->m_xpos-BODY_XSHIFT + shake_dx - 1,
                     thisgo->m_ypos-BODY_YSHIFT + shake_dy - 1,
                     3, 3, PIXEL_BLUE);
#endif
}

static void Tank_render_indicator(GameObject *,
                                  struct pixel,
                                  struct pixel,
                                  int32_t,
                                  uint32_t);

static void Tank_renderm_health_indicator(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    uint32_t width =
        (uint32_t) ((BAR_WIDTH * this->m_health) / TANK_START_HEALTH);

    if (this->m_health > 0 && width == 0)
        width = 1;

    Tank_render_indicator(thisgo,
                          PIXEL_RED, PIXEL_GREEN,
                          HEALTH_BAR_YSHIFT, width);
}

static void Tank_render_charge_indicator(GameObject *thisgo)
{
    Tank *this = (Tank*) thisgo;

    uint32_t width =
        (uint32_t) ((BAR_WIDTH * this->m_turret_charge) / TURRET_MAX_CHARGE);

    Tank_render_indicator(thisgo,
                          PIXEL_GREY, PIXEL_WHITE,
                          CHARGE_BAR_YSHIFT, width);
}

static void Tank_render_indicator(GameObject *thisgo,
                                  struct pixel back_color,
                                  struct pixel front_color,
                                  int32_t y_shift,
                                  uint32_t frontm_width)
{
    int32_t x = thisgo->m_xpos - BAR_WIDTH/2;
    int32_t y = MAX(0, thisgo->m_ypos + y_shift);

    screen_draw_rect(x, y,
                     BAR_WIDTH, BAR_HEIGHT,
                     back_color);

    screen_draw_rect(x, y,
                     frontm_width, BAR_HEIGHT,
                     front_color);
}
