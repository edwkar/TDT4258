#include "ap7000.h"
#include "leds.h"

#define LEDS_PIO             AVR32_PIOB
#define LEDS_ALL             ((1<<LEDS_CNT)-1)

static volatile avr32_pio_t *leds_pio = &LEDS_PIO;

void leds_init(void)
{
    leds_pio = &LEDS_PIO;

    /* Enable LEDs.
     */
    leds_pio->per = LEDS_ALL;
    leds_pio->oer = LEDS_ALL;

    /* Initially turn all off.
     */
    leds_set(0);
}

void leds_set(unsigned int config)
{
    leds_pio->sodr = config;
    leds_pio->codr = ~config;
}
