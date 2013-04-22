#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "profiling.h"
#include "screen.h"
#include "sprite.h"
#include "utils.h"

#include "Terrain.h"


#define IMPACT_RANGE     20
// #define MIN_Y_FOR_IMPACT 10

static void Terrain_destruct(GameObject *thisgo);
static void Terrain_render(GameObject *thisgo);
static void Terrain_init_grid(Terrain *this);
static int32_t Terrain_height_at(const Terrain *this, int32_t xpos);
static void Terrain_apply_impact(GameObject *thisgo, int32_t m_x, int32_t m_y);
static void Terrain_regenerate_sprite(Terrain *this);

Terrain * Terrain_construct(uint32_t width, uint32_t height)
{
    Terrain *this = malloc_or_die(sizeof(Terrain));
    Terrain_init(this, width, height);
    return this;
}

void Terrain_init(Terrain *this, uint32_t width, uint32_t height)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->destruct = Terrain_destruct;
    thisgo->render = Terrain_render;
    thisgo->apply_impact = Terrain_apply_impact;

    this->height_at = Terrain_height_at;

    this->m_width = width;
    this->m_height = height;

    thisgo->m_xpos = 0;
    thisgo->m_ypos = 0;

    this->m_grid = malloc_or_die(width * height * sizeof(uint8_t));
    this->_bg_sprite = sprite_load(SC_RESOURCES_PATH "war.gif.image");
    this->_sprite = sprite_construct(width, height);

    Terrain_init_grid(this);
    Terrain_regenerate_sprite(this);
}

static void Terrain_destruct(GameObject *thisgo)
{
    Terrain *this = (Terrain*) thisgo;

    free(this->m_grid);
    sprite_free(this->_bg_sprite);
    sprite_free(this->_sprite);

    free(thisgo);
}

static void Terrain_init_grid(Terrain *this)
{
    for (uint32_t y = 0, idx = 0; y < this->m_height; ++y)
        for (uint32_t x = 0; x < this->m_width; ++x, ++idx) {
            uint32_t edge = (uint32_t) MAX(80, -0.01*pow((float)x-140.0, 2)+120);
            this->m_grid[idx] = y < edge;
        }
}

static void Terrain_regenerate_sprite(Terrain *this)
{
    for (uint32_t y = 0, idx = 0; y < this->m_height; ++y)
        for (uint32_t x = 0; x < this->m_width; ++x, ++idx) {
            struct pixel p;
            if (this->m_grid[idx])
                p = PIXEL_BLACK;
            else
                p = sprite_get_pixel(this->_bg_sprite, x, y);

            sprite_set_pixel(this->_sprite, x, y, p);
        }
}

static int32_t Terrain_height_at(const Terrain *this, int32_t xpos)
{
    bool is_outside = xpos < 0 || xpos >= (int32_t) this->m_width;
    if (is_outside)
        return 0;

    for (uint32_t y = this->m_height-1; y > 0; --y) {
        uint32_t idx = y*this->m_width + (uint32_t) xpos;
        if (this->m_grid[idx])
            return (int32_t) y;
    }
    return 0;
}

// XXX OMFG REFACTOR THIS.
static void Terrain_apply_impact(GameObject *thisgo, int32_t m_x, int32_t m_y)
{
    Terrain *this = (Terrain*) thisgo;

    for (int32_t x = m_x-2*IMPACT_RANGE; x < m_x+2*IMPACT_RANGE; ++x)
        for (int32_t y = m_y-2*IMPACT_RANGE; y < m_y+2*IMPACT_RANGE; ++y) {
            /*
            bool dist_less = (x-m_x)*(x-m_x) + (y-m_y)*(y-m_y) <=
                             2*IMPACT_RANGE*IMPACT_RANGE;
            if (x >= 0 && y >= MIN_Y_FOR_IMPACT &&
                x < (int32_t) this->m_width && y < (int32_t) this->m_height
                && dist_less)
                this->m_grid[y*this->m_width + x] = false;
                */
        }

    Terrain_regenerate_sprite(this);
}

PROFILING_SETUP(Terrain_render);
static void Terrain_render(GameObject *thisgo)
{
    PROFILING_ENTER(Terrain_render);

    Terrain *this = (Terrain*) thisgo;
    assert(this->_sprite != NULL);
    screen_draw_sprite_raw(this->_sprite);

    PROFILING_EXIT(Terrain_render);
}
