#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <sys/interrupts.h>
#include <sys/queue.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "switches.h"
#include "leds.h"
#include "utils.h"

#define ASSERT_VALID_LED(n)  assert((n) < LEDS_CNT)
#define LEDS_PIO             AVR32_PIOC
#define LEDS_ALL             ((1<<LEDS_CNT)-1)

static bool __module_is_inited = false;

static volatile avr32_pio_t *leds_pio = &LEDS_PIO;

void leds_init(void)
{
    assert(!__module_is_inited);

    leds_pio = &LEDS_PIO;

    /* Enable LEDs.
     */
    leds_pio->per = LEDS_ALL;
    leds_pio->oer = LEDS_ALL;

    __module_is_inited = true;

    /* Initially turn all off.
     */
    leds_turn_off_all();
}

/* Light the single LED <led>.
 */
void leds_light(uint32_t led)
{
    assert(__module_is_inited);
    ASSERT_VALID_LED(led);

    leds_pio->sodr |= 1U << led;
}

void leds_light_all(void)
{
    assert(__module_is_inited);

    leds_pio->sodr = LEDS_ALL;
}

/* Turn off the single LED <led>.
 */
void leds_turn_off(uint32_t led)
{
    assert(__module_is_inited);
    ASSERT_VALID_LED(led);

    leds_pio->codr = 1U << led;
}

void leds_turn_off_all(void)
{
    assert(__module_is_inited);

    leds_pio->codr = LEDS_ALL;
}

/* Light LED <led> if <cond> is true.
 */
void leds_lighted_if(uint32_t led, bool cond) {
    assert(__module_is_inited);

    if (cond)
        leds_light(led);
    else
        leds_turn_off(led);
}
