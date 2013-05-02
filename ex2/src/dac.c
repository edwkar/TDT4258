#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <sys/interrupts.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dac.h"
#include "utils.h"

#define ABDAC_PIO           AVR32_PIOB
#define ABDAC_INT_PRIO      3

static bool module_is_inited = false;

static volatile avr32_abdac_t *dac = &AVR32_ABDAC;

/* Data structures for a basic double buffer.
 */
static int16_t buf_a[DAC_BUF_SIZE];
static int16_t buf_b[DAC_BUF_SIZE];

static volatile int16_t *front = buf_a;
static volatile int16_t front_pos = DAC_BUF_SIZE;
static volatile int16_t *back = buf_b;
static volatile int16_t has_back = false;

bool dac_may_write()
{
    assert(module_is_inited);
    return !has_back;
}

void dac_write_buffer(const int16_t buf[static DAC_BUF_SIZE])
{
    assert(module_is_inited);
    assert(dac_may_write());

    memcpy((void *) back, buf, DAC_BUF_SIZE * sizeof(int16_t));
    has_back = true;
}

static void dac_handle_interrupt();

void dac_init(void)
{
    assert(!module_is_inited);

    /* First, register the ABDAC interrupt handler.
     */
    register_interrupt(dac_handle_interrupt,
                       AVR32_ABDAC_IRQ / 32,
                       AVR32_ABDAC_IRQ % 32, ABDAC_INT_PRIO);

    /* Disable PIO driving PIOB outputs 20 and 21, and let instead
     * Peripheral A -- that is, the ABDAC -- control these two pins.
     */
    volatile avr32_pio_t *dac_pio = &ABDAC_PIO;
    dac_pio->PDR.p20 = 1;
    dac_pio->PDR.p21 = 1;
    dac_pio->ASR.p20 = 1;
    dac_pio->ASR.p21 = 1;

    /* Now, setup a clock source for the ABDAC.
     */
    volatile avr32_pm_t *pm = &AVR32_PM;  // (Power manager.)
    pm->GCCTRL[6].pllsel = 0;             // Use oscillator...
    pm->GCCTRL[6].oscsel = 1;             //       ...number 1
    pm->GCCTRL[6].diven = 0;              // Disable clock division.
    pm->GCCTRL[6].cen = 1;                // Enable the clock.

    /* Finally, enable the ABDAC, by setting the "en"-bit in its Control
     * Register, and interrupts by setting the "tx_ready" bit in its Interrupt
     * Enable Register.
     */
    dac->CR.en = 1;
    dac->IER.tx_ready = 1;

    module_is_inited = true;
}

static void dac_handle_interrupt(void)
{
    assert(module_is_inited);

    int16_t v = front_pos != DAC_BUF_SIZE ? front[front_pos++] : 0;
    dac->SDR.channel0 = v;
    dac->SDR.channel1 = v;

    bool may_swap_buffers = front_pos == DAC_BUF_SIZE && has_back;
    if (may_swap_buffers) {
        volatile int16_t *t = front;
        front = back;
        back = t;
        front_pos = 0;
        has_back = false;
    }
}
