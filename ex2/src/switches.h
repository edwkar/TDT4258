#ifndef __SWITCHES_H
#define __SWITCHES_H

#define SWITCHES_CNT                8
#define SWITCH_FROM_LEFT(i)         (1U << (SWITCHES_CNT-1-(i)))
#define SWITCH_FROM_RIGHT(i)        (1U << (i))
#define SWITCH_IS_PRESSED(state, s) ((state) & (s))

#include <stdint.h>

typedef void (*switch_act_listener)(uint32_t);

void switches_init();
void switches_use_listener(switch_act_listener listener);

#endif 
