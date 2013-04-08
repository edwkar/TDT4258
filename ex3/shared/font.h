#ifndef __FONT_H
#define __FONT_H

#include <stdint.h>

struct font;

struct font * load_font(const char *path);
void font_free(struct font *font);
void font_render(struct font *f, uint16_t _x, uint16_t _y, const char *msg);

#endif
