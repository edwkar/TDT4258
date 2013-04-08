#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "screen.h"
#include "utils.h"
#include "Projectile.h"

#define PROJ_WIDTH  4
#define PROJ_HEIGHT 4
#define PROJ_XSHIFT PROJ_WIDTH/2
#define PROJ_YSHIFT PROJ_HEIGHT/2
#define Y_ACC       -0.2

static void Projectile_update_physics(GameObject* thisgo);
static void Projectile_destruct(GameObject *thisgo);
static void Projectile_render(GameObject *thisgo);
static void Projectile_fire_from(struct _Projectile *this, uint16_t x,
        uint16_t y, uint16_t angle, double power);
static bool Projectile_has_landed(const struct _Projectile *this,
        uint16_t landing_pos[static 2]);
static void Projectile_reset(struct _Projectile *this);

Projectile * Projectile_construct(Terrain *terrain)
{
    Projectile *this = malloc_or_die(sizeof(Projectile));
    Projectile_init(this, terrain);
    return this;
}

void Projectile_init(Projectile *this, Terrain *terrain)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->destruct = Projectile_destruct;
    thisgo->update_physics = Projectile_update_physics;
    thisgo->render = Projectile_render;

    this->reset = Projectile_reset;
    this->fire_from = Projectile_fire_from;
    this->has_landed = Projectile_has_landed;

    this->_terrain = terrain;

    this->reset(this);

    this->_sprite = sprite_construct(PROJ_WIDTH, PROJ_HEIGHT);
    for (int i = 0; i < PROJ_WIDTH*PROJ_HEIGHT; ++i)
        this->_sprite->data[i] = PIXEL_RED;
}

static void Projectile_destruct(GameObject *thisgo)
{
    Projectile *this = (Projectile*) thisgo;
    sprite_free(this->_sprite);
    free(thisgo);
}

static void Projectile_reset(struct _Projectile *this)
{
    this->_is_active = false;
    this->_x = 0;
    this->_y = 0;
    this->_vy = 0;
    this->_vx = 0;
}

static void Projectile_render(GameObject *thisgo) {
    Projectile *this = (Projectile*) thisgo;
    if (!this->_is_active)
        return;

    screen_draw_sprite(this->_x+PROJ_XSHIFT,
                       this->_y+PROJ_YSHIFT,
                       this->_sprite);
}

static void Projectile_fire_from(struct _Projectile *this, uint16_t x,
        uint16_t y, uint16_t angle, double power)
{
    this->_x = x;
    this->_y = y;
    this->_vx = power * cos(angle / 180.0 * M_PI);
    this->_vy = power * sin(angle / 180.0 * M_PI);
    this->_is_active = true;

    printf("%d %d %d %f %f\n", angle, x, y, this->_vx, this->_vy);
}

static void Projectile_update_physics(GameObject* thisgo) {
    Projectile *this = (Projectile*) thisgo;
    if (!this->_is_active)
        return;

    this->_x += this->_vx;
    this->_y += this->_vy;
    this->_vx += 0; /* wind? XXX */
    this->_vy += Y_ACC;

    this->_x = clamp(0, this->_x, FB_WIDTH);
    uint16_t ter_height = this->_terrain->height_at(this->_terrain,
                                                    this->_x);
    this->_y = max(this->_y, ter_height);
}

static bool Projectile_has_landed(const struct _Projectile *this,
        uint16_t landing_pos[static 2])
{
    uint16_t ter_height = this->_terrain->height_at(this->_terrain, this->_x);

    if (ter_height < this->_y)
        return false;
    else {
        landing_pos[0] = (uint16_t) this->_x;
        landing_pos[1] = ter_height;
        return true;
    }
}
