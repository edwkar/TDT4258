#include "ap7000.h"
#include "leds.h"


#define LEDS_PIO             AVR32_PIOB
#define LEDS_PIO_SHIFT       8
#define LEDS_ALL             (0xFFFFFFU << LEDS_PIO_SHIFT)

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

void leds_set(unsigned int c)
{
    unsigned int v = 0;

    if (c & (1U << 0)) v |= 1U <<  0;
    if (c & (1U << 1)) v |= 1U <<  1;
    if (c & (1U << 2)) v |= 1U <<  2;
    if (c & (1U << 3)) v |= 1U <<  5;
    if (c & (1U << 4)) v |= 1U <<  6;
    if (c & (1U << 5)) v |= 1U <<  7;
    if (c & (1U << 6)) v |= 1U <<  8;
    if (c & (1U << 7)) v |= 1U << 22;

    leds_pio->codr = LEDS_ALL;
    leds_pio->sodr = v << LEDS_PIO_SHIFT;
}
