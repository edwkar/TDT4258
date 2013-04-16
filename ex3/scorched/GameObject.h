#ifndef __GAME_OBJECT_H
#define __GAME_OBJECT_H

#include <stdint.h>

#include "sprite.h"


typedef struct _GameObject {
    void (*destruct)(struct _GameObject *this);

    void (*reset)(struct _GameObject *this);

    void (*update)(struct _GameObject *this);
    void (*apply_impact)(struct _GameObject *this, int32_t x, int32_t y);

    void (*render)(struct _GameObject *this);

    struct sprite * (*get_sprite)(struct _GameObject *this);

    int32_t _xpos;
    int32_t _ypos;
} GameObject;

void GameObject_init(GameObject *this);

#define FOR_EACH_GAME_OBJECT(XS, METH_NAME, ...) \
    for (int __i = 0, __n = sizeof XS / sizeof XS[0]; __i < __n; ++__i) \
        if (XS[__i] && XS[__i]->METH_NAME != NULL) \
            XS[__i]->METH_NAME(XS[__i], ##__VA_ARGS__);

#endif
