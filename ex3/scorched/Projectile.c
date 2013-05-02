#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "screen.h"
#include "utils.h"

#include "Projectile.h"


#define PROJ_WIDTH  4
#define PROJ_HEIGHT 4
#define PROJ_XSHIFT (PROJ_WIDTH/2)
#define PROJ_YSHIFT (PROJ_HEIGHT/2)
#define Y_ACC       -0.2

static void Projectile_update(GameObject *);
static void Projectile_destruct(GameObject *);
static void Projectile_render(GameObject *);
static void Projectile_fire_from(struct m_Projectile *, int32_t,
                                 int32_t, uint32_t, float);
static bool Projectile_has_landed(const struct m_Projectile *,
                                  int32_t[]);
static void Projectile_reset(struct m_Projectile *);


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
    thisgo->update = Projectile_update;
    thisgo->render = Projectile_render;
    this->reset = Projectile_reset;
    this->fire_from = Projectile_fire_from;
    this->has_landed = Projectile_has_landed;

    this->_terrain = terrain;

    this->reset(this);

    /* We'll just draw the projectile as a red rectangle.
     */
    this->_sprite = sprite_construct(PROJ_WIDTH, PROJ_HEIGHT);
    for (uint32_t x = 0; x < PROJ_WIDTH; ++x)
        for (uint32_t y = 0; y < PROJ_HEIGHT; ++y)
            sprite_set_pixel(this->_sprite, x, y, PIXEL_RED);
}

static void Projectile_destruct(GameObject *thisgo)
{
    Projectile *this = (Projectile*) thisgo;
    sprite_free(this->_sprite);
    free(thisgo);
}

static void Projectile_reset(struct m_Projectile *this)
{
    this->m_is_active = false;
    this->m_x = 0;
    this->m_y = 0;
    this->m_vy = 0;
    this->m_vx = 0;
}

static void Projectile_render(GameObject *thisgo) {
    Projectile *this = (Projectile*) thisgo;

    if (!this->m_is_active)
        return;

    screen_draw_sprite((int32_t) (this->m_x + PROJ_XSHIFT),
                       (int32_t) (this->m_y + PROJ_YSHIFT),
                       this->_sprite);
}

static void Projectile_fire_from(struct m_Projectile *this, int32_t x,
                                 int32_t y, uint32_t angle, float power)
{
    this->m_x = x;
    this->m_y = y;
    this->m_vx = power * cosf(angle / 180.0F * M_PI_F);
    this->m_vy = power * sinf(angle / 180.0F * M_PI_F);
    this->m_is_active = true;
}

static void Projectile_update(GameObject* thisgo)
{
    Projectile *this = (Projectile*) thisgo;

    if (!this->m_is_active)
        return;

    this->m_x += this->m_vx;
    this->m_y += this->m_vy;
    this->m_vx += 0; /* wind? XXX */
    this->m_vy += Y_ACC;

    int32_t ter_height =
        this->_terrain->height_at(this->_terrain, (int32_t) this->m_x);

    this->m_y = MAX(this->m_y, ter_height);
}

static bool Projectile_has_landed(const struct m_Projectile *this,
                                  int32_t landing_pos[static 2])
{
    int32_t ter_height =
        this->_terrain->height_at(this->_terrain, (int32_t) this->m_x);

    if (ter_height < this->m_y)
        return false;
    else {
        landing_pos[0] = (int32_t) this->m_x;
        landing_pos[1] = ter_height;
        return true;
    }
}
