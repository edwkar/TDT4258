#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "screen.h"
#include "sprite.h"

#include "GameObject.h"


static void GameObject_render(GameObject *);

void GameObject_init(GameObject *this)
{
    /* Initialize vtable.
     */
    this->destruct = NULL;
    this->apply_impact = NULL;
    this->update = NULL;
    this->reset = NULL;
    this->get_sprite = NULL;
    this->render = GameObject_render;

    /* Initialize common properties.
     */
    this->m_xpos = 0;
    this->m_ypos = 0;
}

static void GameObject_render(GameObject *this)
{
    if (this->get_sprite == NULL)
        return;

    struct sprite *s = this->get_sprite(this);
    if (s == NULL)
        return;

    screen_draw_sprite(this->m_xpos, this->m_ypos, s);
}
