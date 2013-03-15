#ifndef __CONTROLLER_H
#define __CONTROLLER_H 

#include <stdbool.h>
#include "synthesizer.h"

#define CONTROLLER_OBJECT_POOL_SIZE 8
#define CONTROLLER_MAX_NUM_TRACKS 8

struct controller;

struct controller *controller_construct(void);
void controller_add_track(struct controller *c, struct synthesis *s);
void controller_advance(struct controller *s, uint32_t num_to_write, 
                        int16_t res[static num_to_write]);
void controller_start_track(struct controller *c, uint32_t track_num,
                            bool should_loop);
void controller_stop_track(struct controller *c, uint32_t track_num);
bool controller_track_is_active(struct controller *c, uint32_t track_num);
bool controller_track_is_looping(struct controller *c, uint32_t track_num);

#endif
