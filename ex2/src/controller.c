#include <assert.h>
#include <stdlib.h>
#include "controller.h"
#include "synthesizer.h"

struct controller {
    uint32_t num_tracks;
    uint32_t active_tracks;  // Bit mask.
    uint32_t looping_tracks; // Bit mask.
    struct synthesis *_tracks[CONTROLLER_MAX_NUM_TRACKS];
};

static void assert_valid_track_num(const struct controller *c, uint32_t n)
{
    assert(n < c->num_tracks);
}


/* struct controller object pool.
 */
static struct controller object_pool[CONTROLLER_OBJECT_POOL_SIZE];
static int pool_num_created = 0;

struct controller *controller_construct(void)
{
    assert(pool_num_created < CONTROLLER_OBJECT_POOL_SIZE);

    struct controller *c = &object_pool[pool_num_created++];
    c->num_tracks = 0;
    c->active_tracks = 0;
    c->looping_tracks = 0;

    return c;
}

void controller_add_track(struct controller *c, struct synthesis *s)
{
    assert(c->num_tracks < CONTROLLER_MAX_NUM_TRACKS);
    c->_tracks[c->num_tracks++] = s;
}

void controller_advance(struct controller *c,
                        uint32_t num_to_write,
                        int16_t res[static num_to_write])
{

    for (uint32_t i = 0; i < c->num_tracks; ++i) {
        if (!controller_track_is_active(c, i))
            continue;

        struct synthesis *synth = c->_tracks[i];

        uint32_t num_written = 0;
        while (num_written < num_to_write) {
            if (synthesis_is_at_end(synth)) {
                if (controller_track_is_looping(c, i))
                    synthesis_rewind(synth);
                else {
                    controller_stop_track(c, i);
                    break;
                }
            }

            uint32_t nw = synthesis_advance(synth,
                                            num_to_write-num_written,
                                            res+num_written);
            num_written += nw;
        }
    }
}

void controller_start_track(struct controller *c, uint32_t track_num,
                            bool should_loop)
{
    assert_valid_track_num(c, track_num);

    synthesis_rewind(c->_tracks[track_num]);
    c->active_tracks |= 1U << track_num;

    if (should_loop)
        c->looping_tracks |= 1U << track_num;
    else
        c->looping_tracks &= ~(1U << track_num);
}

void controller_stop_track(struct controller *c, uint32_t track_num)
{
    assert_valid_track_num(c, track_num);
    c->active_tracks &= ~(1U << track_num);
}

bool controller_track_is_active(struct controller *c, uint32_t track_num)
{
    assert_valid_track_num(c, track_num);
    return c->active_tracks & (1U << track_num);
}

bool controller_track_is_looping(struct controller *c, uint32_t track_num)
{
    assert_valid_track_num(c, track_num);
    return c->looping_tracks & (1U << track_num);
}
