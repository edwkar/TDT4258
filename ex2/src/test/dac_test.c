#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <stdlib.h>
#include <sys/interrupts.h>
#include "dac.h"

int16_t buf[DAC_BUF_SIZE];

int main(void) {
    set_interrupts_base((void *) AVR32_INTC_ADDRESS);

    dac_init();

    init_interrupts();

    for (;;) {
        while (!dac_may_write())
            ;
        for (int i = 0; i < DAC_BUF_SIZE; ++i)
            buf[i] = rand();
        dac_write_buffer(buf);
    }

    return 0;
}
