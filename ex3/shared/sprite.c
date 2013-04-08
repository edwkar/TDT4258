#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "screen.h"
#include "sprite.h"
#include "utils.h"

struct sprite * sprite_construct(uint16_t width, uint16_t height) {
    struct sprite *s = malloc_or_die(sizeof(struct sprite));
    s->width = width;
    s->height = height;
    s->data = malloc_or_die(width * height * sizeof(struct pixel));
    memset(s->data, 255, width * height * sizeof(struct pixel));
    return s;
}

struct sprite * sprite_load(const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL)
        die_hard("fopen");

    uint16_t sprite_sz[2];
    if (fread(sprite_sz, 2, 2, f) != 2)
        die_hard("fread");
    uint16_t w = sprite_sz[0],
             h = sprite_sz[1];

    bool is_big_endian = htonl(47) == 47;
    if (is_big_endian) {
        w = (w >> 8) | (w << 8);
        h = (h >> 8) | (h << 8);
    }
    printf("%u %u\n", w, h);

    uint8_t data[w * h * 4];
    if (fread(data, 1, sizeof data, f) != sizeof data)
        die_hard("fread");

    if (!(fgetc(f) == EOF && feof(f)))
        die_hard("");

    if (fclose(f) != 0)
        die_hard("fclose");

    struct sprite *s = sprite_construct(w, h);
    for (uint32_t x = 0, idx = 0; x < s->width; ++x)
        for (uint32_t y = 0; y < s->height; ++y, ++idx) {
            bool is_transparent = data[4*idx+3] != 0;
            if (is_transparent)
                s->data[idx] = PIXEL_TRANSPARENT;
            else {
                s->data[idx].red   = data[4*idx+0];
                s->data[idx].green = data[4*idx+1];
                s->data[idx].blue  = data[4*idx+2];
            }
        }

    printf("Loaded sprite from %s.\n", path);
    return s;
}

void sprite_invert_horizontal(struct sprite *s) {
    for (int x = 0; x < s->width/2; ++x)
        for (uint32_t y = 0; y < s->height; ++y) {
            uint32_t aidx =             x*s->height+y;
            uint32_t bidx = (s->width-1-x)*s->height+y;
            struct pixel t = s->data[aidx];
            s->data[aidx] = s->data[bidx];
            s->data[bidx] = t;
        }
}

void sprite_free(struct sprite *s) {
    free(s->data);
    free(s);
}
