#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "screen.h"
#include "GameObject.h"
#include "sprite.h"

static void GameObject_render(GameObject *this);

void GameObject_init(GameObject *this)
{
    this->destruct = NULL;
    this->apply_impact = NULL;
    this->update_physics = NULL;
    this->reset = NULL;
    this->render = GameObject_render;
    this->get_sprite = NULL;
    this->_xpos = 100;
    this->_ypos = 20;
}

static void GameObject_render(GameObject *this)
{
    if (this->get_sprite == NULL)
        return;

    const struct sprite *s = this->get_sprite(this);
    if (s == NULL)
        return;

    screen_draw_sprite(this->_xpos, this->_ypos, s);
}
