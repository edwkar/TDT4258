#ifndef mGAME_OBJECT_H
#define mGAME_OBJECT_H

#include <stdint.h>

#include "sprite.h"


typedef struct m_GameObject {
    void (*destruct)(struct m_GameObject *this);

    void (*reset)(struct m_GameObject *this);

    void (*update)(struct m_GameObject *this);
    void (*apply_impact)(struct m_GameObject *this, int32_t x, int32_t y);

    void (*render)(struct m_GameObject *this);

    struct sprite * (*get_sprite)(struct m_GameObject *this);

    int32_t m_xpos;
    int32_t m_ypos;
} GameObject;

void GameObject_init(GameObject *this);

#define FOR_EACH_GAME_OBJECT(XS, METH_NAME, ...) \
    for (int mi = 0, mn = sizeof XS / sizeof XS[0]; mi < mn; ++mi) \
        if (XS[mi] && XS[mi]->METH_NAME != NULL) \
            XS[mi]->METH_NAME(XS[mi], ##__VA_ARGS__);

#endif
