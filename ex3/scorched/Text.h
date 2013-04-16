#ifndef __TEXT_H
#define __TEXT_H

#include <stdbool.h>
#include <stdint.h>

#include "GameObject.h"


#define TEXT_MAX_CNT           16
#define TEXT_MAX_LEN           128 

#define TEXT_CENTER_HORIZONTAL -1

/* PRIVATE */
struct _TextEntry {
    int32_t x;
    int32_t y;
    char msg[TEXT_MAX_LEN+1];
    uint32_t time_end_ms;
};

typedef struct _Text {
    GameObject _base;

    bool (*add)(struct _Text *this, int32_t x, int32_t y,
                const char *text, const uint32_t time_ms);

    uint32_t _active_entries;
    struct _TextEntry _entries[TEXT_MAX_CNT];

    struct sprite *_font_sprite;
    struct sprite *_out_sprite;
    bool _need_update;

    int32_t _phase;

    uint32_t _MIN_y_needed;
    uint32_t _MAX_y_needed;
} Text;

Text * Text_construct(void);
void Text_init(Text *this);

#endif
