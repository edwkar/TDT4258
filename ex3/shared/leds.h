#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

void leds_init(void);
void leds_set(uint8_t config);
void leds_cleanup(void);

#endif
