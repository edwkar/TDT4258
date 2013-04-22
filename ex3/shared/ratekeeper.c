#include <stdio.h>
#include <unistd.h>

#include "profiling.h"
#include "ratekeeper.h"
#include "utils.h"


#define RATE_KEEPER_CNTS 5

static uint32_t t = 0;

struct rate_keeper rate_keeper_construct(uint32_t intended_fps)
{
    return (struct rate_keeper) {
        .intended_fps = intended_fps,
        .time_begin = get_time_ms(),
        .num_cnts = -1,
#ifdef HOST_BUILD
        .sleep_time_ms = 40
#else
        .sleep_time_ms = 20
#endif
    };
}

static void rate_keeper_update_sleep_time(struct rate_keeper *rk) {
    int64_t time_now = get_time_ms();

    uint32_t avg_fps = (uint32_t) (1000.0*RATE_KEEPER_CNTS /
                                     (time_now - rk->time_begin));

    if (avg_fps < rk->intended_fps)
        rk->sleep_time_ms--;
    else if (avg_fps > rk->intended_fps)
        rk->sleep_time_ms++;

    if (rk->sleep_time_ms < 0)
        rk->sleep_time_ms = 0;

    if ((t++)%5 == 0) {
        printf("     AVG FPS: %d. Sleeping %u ms\n", (int) avg_fps,
                                   rk->sleep_time_ms);
        profiling_report();
        puts("");
    }
}

void rate_keeper_tick(struct rate_keeper *rk)
{
    if (rk->num_cnts == RATE_KEEPER_CNTS) {
        rate_keeper_update_sleep_time(rk);
        rk->time_begin = get_time_ms();
        rk->num_cnts = -1;
    }

    if (rk->sleep_time_ms != 0)
        usleep((unsigned int) (1000 * rk->sleep_time_ms));

    rk->num_cnts++;
}
