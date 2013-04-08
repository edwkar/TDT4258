#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "screen.h"
#include "utils.h"
#include "Terrain.h"

#define IMPACT_RANGE     20
#define MIN_Y_FOR_IMPACT 50

static void Terrain_destruct(GameObject *thisgo);
static const struct sprite * Terrain_get_sprite(GameObject *thisgo);
static void Terrain_init_grid(Terrain *this);
static uint16_t Terrain_height_at(const Terrain *this, uint16_t xpos);
static void Terrain_apply_impact(GameObject *thisgo, int32_t _x, int32_t _y);
static void Terrain_regenerate_sprite(Terrain *this);

Terrain * Terrain_construct(uint16_t width, uint16_t height)
{
    Terrain *this = malloc_or_die(sizeof(Terrain));
    Terrain_init(this, width, height);
    return this;
}

void Terrain_init(Terrain *this, uint16_t width, uint16_t height)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->destruct = Terrain_destruct;
    thisgo->get_sprite = Terrain_get_sprite;
    thisgo->apply_impact = Terrain_apply_impact;
    thisgo->_xpos = 0;
    thisgo->_ypos = 0;
    this->height_at = Terrain_height_at;

    this->_width = width;
    this->_height = height;

    this->_grid = malloc_or_die(width * height * sizeof(uint8_t));
    this->_sprite = sprite_construct(width, height);

    Terrain_init_grid(this);
    Terrain_regenerate_sprite(this);
}

static void Terrain_init_grid(Terrain *this)
{
    for (uint32_t x = 0, idx = 0; x < this->_width; ++x)
        for (uint32_t y = 0; y < this->_height; ++y, ++idx) {
            uint32_t edge = max(80, -0.01*pow((float)x-140.0, 2)+120);
            this->_grid[idx] = y < edge;
        }
}

static uint16_t Terrain_height_at(const Terrain *this, uint16_t xpos)
{
    for (uint32_t y = this->_height-1; y > 0; --y) {
        uint32_t idx = xpos*this->_height + y;
        if (this->_grid[idx])
            return y;
    }
    return 0;
}

static void Terrain_regenerate_sprite(Terrain *this) {
    for (uint32_t x = 0, idx = 0; x < this->_width; ++x)
        for (uint32_t y = 0; y < this->_height; ++y, ++idx)
            this->_sprite->data[idx] =
                this->_grid[idx] ? PIXEL_BROWN : PIXEL_BLACK;
}

static void Terrain_apply_impact(GameObject *thisgo, int32_t _x, int32_t _y)
{
    Terrain *this = (Terrain*) thisgo;
    for (int32_t x = _x-2*IMPACT_RANGE; x < _x+2*IMPACT_RANGE; ++x)
        for (int32_t y = _y-2*IMPACT_RANGE; y < _y+2*IMPACT_RANGE; ++y) {
            bool dist_less = (x-_x)*(x-_x) + (y-_y)*(y-_y) <=
                             2*IMPACT_RANGE*IMPACT_RANGE;
            if (x >= 0 && y >= MIN_Y_FOR_IMPACT &&
                x < (int32_t) this->_width && y < (int32_t) this->_height
                && dist_less)
                this->_grid[x*this->_height + y] = false;
        }
    Terrain_regenerate_sprite(this);
}

static void Terrain_destruct(GameObject *thisgo) {
    Terrain *this = (Terrain*) thisgo;
    free(this->_grid);
    sprite_free(this->_sprite);
    free(thisgo);
}

static const struct sprite * Terrain_get_sprite(GameObject *thisgo) {
    Terrain *this = (Terrain*) thisgo;
    assert(this->_sprite != NULL);
    return this->_sprite;
}
