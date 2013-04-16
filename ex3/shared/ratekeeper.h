#ifndef __RATE_KEEPER_H
#define __RATE_KEEPER_H

#include <stdint.h>


#define RATE_KEEPER_LOG_LENGTH 32

struct rate_keeper {
    uint32_t intended_fps;
    int64_t time_begin;
    int num_cnts;
    int sleep_time_ms;
};

struct rate_keeper rate_keeper_construct(uint32_t intended_fps);
void rate_keeper_tick(struct rate_keeper *rk);

#endif
