#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>

#include "movie.h"
#include "ratekeeper.h"
#include "screen.h"
#include "sprite.h"
#include "utils.h"


#define FFMPEG_PATH         "/usr/bin/ffmpeg"

#define MOVIE_WIDTH         FB_WIDTH
#define MOVIE_HEIGHT        140
#define MOVIE_SHIFT         50*FB_WIDTH

static void run_play_loop(FILE *ffmpeg,
                          struct sprite *out_sprite,
                          size_t num_to_read,
                          struct rate_keeper *rk);

void movie_play(const char *path, struct rate_keeper *rk)
{
    assert(strlen(path) < 256);

    char cmd_line_string[512];
    snprintf(cmd_line_string, sizeof cmd_line_string,
             "%s -i \"%s\" "
             "-f rawvideo -pix_fmt rgb24 -",
             FFMPEG_PATH, path);

    FILE *ffmpeg = popen(cmd_line_string, "r");
    if (ffmpeg == NULL)
        DIE_HARD("popen");

    struct sprite *out_sprite = sprite_construct(FB_WIDTH, FB_HEIGHT);
    for (size_t i = 0; i < FB_WIDTH*FB_HEIGHT; ++i) // XXX fill
        out_sprite->_data[i] = PIXEL_BLACK;

    size_t num_to_read = MOVIE_WIDTH * MOVIE_HEIGHT * sizeof(struct pixel);
    run_play_loop(ffmpeg, out_sprite, num_to_read, rk);

    int ffmpeg_ret = pclose(ffmpeg);
    if (ffmpeg_ret != 0)
        DIE_HARD("pclose");

    sprite_free(out_sprite);
}

void run_play_loop(FILE *ffmpeg,
                   struct sprite *out_sprite,
                   size_t num_to_read,
                   struct rate_keeper *rk)
{
    for (;;) {
        size_t num_read_tot = 0;

        while (num_read_tot != num_to_read) {
            void *pos = out_sprite->_data + MOVIE_SHIFT + num_read_tot;

            size_t nr = fread(pos,
                              1, num_to_read - num_read_tot,
                              ffmpeg);

            if (nr == 0) {
                if (feof(ffmpeg))
                    return;
                else
                    DIE_HARD("fread");
            }

            num_read_tot += (size_t) nr;
        }

        /* Switch the colors in place.
         * (Only has effect on big endian.)
         */
        for (size_t i = 0; i < FB_WIDTH*FB_HEIGHT; ++i) {
            struct pixel *p = &out_sprite->_data[i];
            uint8_t t = p->red;
            p->red = p->blue;
            p->blue = t;
        }

        /* Render.
         */
        screen_draw_sprite_raw(out_sprite);
        screen_redraw();

        rate_keeper_tick(rk);
    }
}
