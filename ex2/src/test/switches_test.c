#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <sys/interrupts.h>
#include "switches.h"
#include "leds.h"
#include "utils.h"

void listener(uint32_t state) {
    for (int i = 0; i < SWITCHES_CNT; ++i)
        leds_lighted_if(LED_FROM_LEFT(i),
                        SWITCH_IS_PRESSED(state, SWITCH_FROM_LEFT(i)));
}

int main(void) {
    set_interrupts_base((void *) AVR32_INTC_ADDRESS);

    leds_init();
    switches_init();

    for (int i = 0; i < SWITCHES_CNT; ++i)
        assert(SWITCH_FROM_LEFT(i) == SWITCH_FROM_RIGHT(SWITCHES_CNT-1-i));

    switches_use_listener(listener);

    init_interrupts();

    for (;;)
        ;

    return 0;
}
