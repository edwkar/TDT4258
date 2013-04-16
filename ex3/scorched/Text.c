#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "profiling.h"
#include "screen.h"
#include "sprite.h"
#include "utils.h"

#include "Text.h"


#define CHAR_WIDTH  15
#define CHAR_HEIGHT 22
#define CHAR_SEP     2

#define TEXT_BUF_WIDTH  FB_WIDTH
#define TEXT_BUF_HEIGHT FB_HEIGHT

static bool Text_add(struct _Text *this, int32_t x, int32_t y,
                     const char *text, const uint32_t time_ms);
static void Text_update(GameObject *thisgo);
static void Text_destruct(GameObject *thisgo);
static void Text_render(GameObject *thisgo);
static bool Text_font_pos(char c, uint32_t *x, uint32_t *y);

Text * Text_construct(void)
{
    Text *this = malloc_or_die(sizeof(Text));
    Text_init(this);
    return this;
}

void Text_init(Text *this)
{
    GameObject *thisgo = (GameObject*) this;
    GameObject_init(thisgo);

    thisgo->destruct = Text_destruct;
    thisgo->update = Text_update;
    thisgo->render = Text_render;

    this->add = Text_add;

    this->_active_entries = 0;
    this->_need_update = true;

    this->_font_sprite = sprite_load(SC_RESOURCES_PATH
                                      "/font/moms_typewriter.gif.image");
    this->_out_sprite = sprite_construct(TEXT_BUF_WIDTH, TEXT_BUF_HEIGHT);

    this->_phase = 0;
}

static void Text_destruct(GameObject *thisgo)
{
    Text *this = (Text*) thisgo;

    sprite_free(this->_font_sprite);
    sprite_free(this->_out_sprite);
    free(thisgo);
}

static bool Text_add(struct _Text *this, int32_t x, int32_t y,
                     const char *text, const uint32_t time_ms)
{
    size_t n = strlen(text);
    if (n >= TEXT_MAX_LEN)
        DIE_HARD("Text too long!");

    int32_t slot_idx = -1;

    for (int32_t i = 0; i < TEXT_MAX_CNT; ++i) {
        bool is_avail = (this->_active_entries & (1U << i)) == 0;
        if (is_avail) {
            slot_idx = i;
            break;
        }
    }

    bool all_slots_are_filled = slot_idx == -1;
    if (all_slots_are_filled)
        return false;

    bool should_center_hor = x == TEXT_CENTER_HORIZONTAL;
    if (should_center_hor) {
        uint32_t req_width = (uint32_t) (n * (CHAR_WIDTH + CHAR_SEP));
        x = TEXT_BUF_WIDTH/2 - req_width/2 + 3*CHAR_WIDTH/4; // Hack
    }

    this->_entries[slot_idx].x = x;
    this->_entries[slot_idx].y = y;
    strncpy(this->_entries[slot_idx].msg, text,
            sizeof this->_entries[slot_idx].msg);
    this->_entries[slot_idx].time_end_ms = get_time_ms() + time_ms;

    this->_active_entries |= 1U << slot_idx;
    this->_need_update = true;

    return true;
}

static void Text_update(GameObject *thisgo)
{
    Text *this = (Text*) thisgo;

    uint32_t t = get_time_ms();

    for (uint32_t i = 0; i < TEXT_MAX_CNT; ++i) {
        bool is_used = (this->_active_entries & (1U << i)) != 0;
        bool is_expired = this->_entries[i].time_end_ms < t;

        /* Should be cleared?
         */
        if (is_used && is_expired) {
            this->_active_entries &= ~(1U << i);
            this->_need_update = true;
        }
    }

    if (!this->_need_update)
        return;

    for (uint32_t x = 0; x < TEXT_BUF_WIDTH; ++x)
        for (uint32_t y = 0; y < TEXT_BUF_HEIGHT; ++y)
            sprite_set_pixel(this->_out_sprite, x, y, PIXEL_TRANSPARENT);

    for (uint32_t i = 0; i < TEXT_MAX_CNT; ++i) {
        bool is_active = (this->_active_entries & (1U << i)) != 0;
        if (!is_active)
            continue;

        const struct _TextEntry *entry = &this->_entries[i];

        int32_t out_x_start = entry->x;
        for (
            size_t j = 0, n = strlen(entry->msg);
            j < n;
            ++j, out_x_start += CHAR_WIDTH + CHAR_SEP
        ) {
            uint32_t in_x_start = 0, in_y_start = 0;
            bool can_draw = Text_font_pos(entry->msg[j],
                                          &in_x_start, &in_y_start);
            if (!can_draw)
                continue;

            uint32_t in_x = in_x_start;
            int32_t out_x = out_x_start;
            for (
                ;
                in_x < in_x_start + CHAR_WIDTH;
                ++in_x, ++out_x
            ) {
                uint32_t in_y = in_y_start;
                int32_t out_y = entry->y;
                for (
                    ;
                    in_y < in_y_start + CHAR_HEIGHT;
                    ++in_y, ++out_y
                ) {
                    if (out_x >= 0 && out_x < TEXT_BUF_WIDTH &&
                        out_y >= 0 && out_y < TEXT_BUF_HEIGHT
                    ) {
                        sprite_set_pixel(this->_out_sprite,
                                         (uint32_t) out_x, (uint32_t) out_y,
                                         sprite_get_pixel(this->_font_sprite,
                                                          in_x, in_y));
                    }
                }
            }
        }
    }

    uint32_t MIN_y_needed = TEXT_BUF_HEIGHT;
    uint32_t MAX_y_needed = 0;
    for (uint32_t x = 0; x < TEXT_BUF_WIDTH; ++x)
        for (uint32_t y = 0; y < TEXT_BUF_HEIGHT; ++y) {
            struct pixel p = sprite_get_pixel(this->_out_sprite, x, y);
            if (!IS_TRANSPARENT(&p)) {
                MIN_y_needed = MIN(y, MIN_y_needed);
                MAX_y_needed = MAX(y+1, MAX_y_needed);
            }
        }
    this->_out_sprite->_y_draw_start = MIN_y_needed;
    this->_out_sprite->_y_draw_end = MAX_y_needed;

    this->_need_update = false;
}

PROFILING_SETUP(Text_render);
static void Text_render(GameObject *thisgo)
{
    PROFILING_ENTER(Text_render);

    Text *this = (Text*) thisgo;

    int32_t x = 0, y = 0;
    if ((this->_phase++%40) < 32)
        x += rand()%4, y += rand()%2;

    screen_draw_sprite(x, y, this->_out_sprite);

    PROFILING_EXIT(Text_render);
}

static bool Text_font_pos(char c, uint32_t *_x, uint32_t *_y)
{
    int32_t x, y;

    if (c == 'A') {
        y = 340;
        x = 5 + 10 * 20;
    } else if (c >= 'B' && c <= 'L') {
        y = 320;
        x = 5 + (c-'B') * 20;
    } else if (c >= 'M' && c <= 'W') {
        y = 300;
        x = 5 + (c-'M') * 20;
    } else if (c >= 'X' && c <= 'Z') {
        y = 280;
        x = 5 + (c-'X') * 20;
    } else if (c >= 'a' && c <= 'b') {
        y = 280;
        x = 5 + (9+c-'a') * 20;
    } else if (c >= 'c' && c <= 'm') {
        y = 260;
        x = 5 + (c-'c') * 20;
    } else if (c >= 'n' && c <= 'x') {
        y = 240;
        x = 5 + (c-'n') * 20;
    } else if (c >= 'y' && c <= 'z') {
        y = 220;
        x = 5 + (c-'y') * 20;
    } else
        return false;

    *_x = (uint32_t) x;
    *_y = (uint32_t) y;
    return true;
}
