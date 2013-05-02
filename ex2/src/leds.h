#ifndef LEDS_H
#define LEDS_H

#include <stdbool.h>
#include <stdint.h>

#define LEDS_CNT          8
#define LED_FROM_LEFT(i)  (LEDS_CNT-1-(i))
#define LED_FROM_RIGHT(i) (i)

void leds_init();
void leds_light(uint32_t led);
void leds_light_all(void);
void leds_turn_off(uint32_t led);
void leds_turn_off_all(void);
void leds_lighted_if(uint32_t led, bool cond);

#endif 
