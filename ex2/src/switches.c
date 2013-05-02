#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <sys/interrupts.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "switches.h"
#include "utils.h"

#define SWITCHES_PIO       AVR32_PIOB
#define SWITCHES_PIO_IRQ   AVR32_PIOB_IRQ
#define SWITCHES_INT_LEVEL 0
#define SWITCHES_ALL       ((1<<SWITCHES_CNT)-1)

static bool module_is_inited = false;

static volatile avr32_pio_t *sw_pio = &SWITCHES_PIO;
static switch_act_listener listener = NULL;

static void switches_handle_interrupt(void);

void switches_init(void)
{
    assert(!module_is_inited);

    sw_pio = &SWITCHES_PIO;

    /*
     * Register interrupt handler.
     */
    register_interrupt(switches_handle_interrupt,
                       SWITCHES_PIO_IRQ / 32,
                       SWITCHES_PIO_IRQ % 32, SWITCHES_INT_LEVEL);

    /*
     * Enable all switches.
     */
    sw_pio->per = SWITCHES_ALL;   // Pin enable register.
    sw_pio->puer = SWITCHES_ALL;  // Pull up enable register.
    sw_pio->ier = SWITCHES_ALL;   // Interrupt enable register.

    module_is_inited = true;
}

static void switches_handle_interrupt(void)
{
    assert(module_is_inited);

    /* We *must* guarantee that the ISR is read.
     * We use a "fake check" to ensure that the compiler
     * does output a read.
     */
    if (sw_pio->isr == 0)
        return;

    /* Spin a bit, for debouncing.
     */
    uint32_t active_switches = ~sw_pio->pdsr;
    for (volatile int k = 0x4242; k; --k)
        ;
    active_switches &= ~sw_pio->pdsr;

    if (listener != NULL)
        listener(active_switches);
}

void switches_use_listener(switch_act_listener lst) {
    listener = lst;
}
