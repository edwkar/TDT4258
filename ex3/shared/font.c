#include "font.h"
#include "sprite.h"
#include "utils.h"

#if 0
struct font {
    struct sprite *s;
};

struct font * load_font(const char *path)
{
    struct font *f = malloc_or_die(sizeof(struct font));
    f->s = sprite_load(path);
    return f;
}

void font_free(struct font *f)
{
    sprite_free(f->s);
    free(f);
}

void font_render(struct font *f, uint16_t _x, uint16_t _y, const char *msg)
{
    if (f == 0)
        return;

    if (msg == 0)
        return;
}
#endif
