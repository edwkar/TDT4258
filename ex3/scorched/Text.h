#ifndef TEXT_H
#define TEXT_H

#include <stdbool.h>
#include <stdint.h>

#include "GameObject.h"


#define TEXT_MAX_CNT           16
#define TEXT_MAX_LEN           128 

#define TEXT_CENTER_HORIZONTAL -1

/* PRIVATE */
struct m_TextEntry {
    int32_t x;
    int32_t y;
    char msg[TEXT_MAX_LEN+1];
    uint32_t time_end_ms;
};

typedef struct m_Text {
    GameObject m_base;

    bool (*add)(struct m_Text *this, int32_t x, int32_t y,
                const char *text, const uint32_t time_ms);

    uint32_t m_active_entries;
    struct m_TextEntry m_entries[TEXT_MAX_CNT];

    struct sprite *_font_sprite;
    struct sprite *_out_sprite;
    bool m_need_update;

    int32_t m_phase;

    uint32_t min_y_needed;
    uint32_t max_y_needed;
} Text;

Text * Text_construct(void);
void Text_init(Text *this);

#endif
