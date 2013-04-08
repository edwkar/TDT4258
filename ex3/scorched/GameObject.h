#ifndef __GAME_OBJECT_H
#define __GAME_OBJECT_H

#include <stdint.h>
#include "sprite.h"

typedef struct _GameObject {
    void (*destruct)(struct _GameObject *this);
    void (*update_physics)(struct _GameObject *this);
    void (*apply_impact)(struct _GameObject *this, int32_t x, int32_t y);
    void (*render)(struct _GameObject *this);
    void (*reset)(struct _GameObject *this);
    const struct sprite * (*get_sprite)(struct _GameObject *this);
    int32_t _xpos;
    int32_t _ypos;
} GameObject;

void GameObject_init(GameObject *this);


#endif
