#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"
#include "sprite.h"
#include "utils.h"


struct sprite * sprite_construct(uint32_t width, uint32_t height)
{
    struct sprite *s = malloc_or_die(sizeof(struct sprite));

    s->m_width = width;
    s->m_height = height;

    size_t n = width * height * sizeof(struct pixel);

    s->_data = malloc_or_die(n);
    memset(s->_data, 255, n);

    s->m_y_draw_start = 0;
    s->m_y_draw_end = height;

    return s;
}

struct sprite * sprite_load(const char *path)
{
    FILE *f = fopen_or_die(path, "r");

    uint16_t sprite_sz[2];
    if (fread(sprite_sz, 2, 2, f) != 2)
        DIE_HARD("fread");

    uint16_t w = sprite_sz[0],
             h = sprite_sz[1];

    bool is_big_endian = htonl(47) == 47;
    if (is_big_endian) {
        w = (uint16_t) ((w >> 8) | (w << 8));
        h = (uint16_t) ((h >> 8) | (h << 8));
    }

    uint32_t data_sz = w * h * 4;
    uint8_t *data = malloc_or_die(data_sz);

    if (fread(data, 1, data_sz, f) != data_sz)
        DIE_HARD("fread");

    if (!(fgetc(f) == EOF && feof(f)))
        DIE_HARD("Expected EOF.");

    if (fclose(f) != 0)
        DIE_HARD("fclose");

    struct sprite *s = sprite_construct(w, h);

    for (uint32_t x = 0, idx = 0; x < s->m_width; ++x)
        for (uint32_t y = 0; y < s->m_height; ++y, ++idx) {
            bool is_transparent = data[4*idx+3] != 0;
            struct pixel p = PIXEL(data[4*idx], data[4*idx+1], data[4*idx+2]);

            sprite_set_pixel(s, x, y, is_transparent ? PIXEL_TRANSPARENT : p);
        }

    free(data);
    printf("Loaded %ux%u sprite from %s.\n", w, h, path);

    return s;
}

void sprite_invert_horizontal(struct sprite *s)
{
    for (uint32_t y = 0; y < s->m_height; ++y)
        for (uint32_t x = 0; x < s->m_width/2; ++x) {
            uint32_t xinv = s->m_width-x-1;
            struct pixel t = sprite_get_pixel(s, x, y);
            sprite_set_pixel(s,  x, y, sprite_get_pixel(s, xinv, y));
            sprite_set_pixel(s, xinv, y, t);
        }
}

void sprite_free(struct sprite *s)
{
    free(s->_data);
    free(s);
}
