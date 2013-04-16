#include "ap7000.h"
#include "switches.h"


#define SWITCHES_PIO             AVR32_PIOB
#define SWITCHES_ALL             ((1<<SWITCHES_CNT)-1)

static volatile avr32_pio_t *switches_pio = &SWITCHES_PIO;

void switches_init(void)
{
    switches_pio = &SWITCHES_PIO;

    switches_pio->per = SWITCHES_ALL;
    switches_pio->puer = SWITCHES_ALL;
}

static volatile int v;
unsigned char switches_read(void)
{
    unsigned char res = ~switches_pio->pdsr;
    for (v = 0; v < 1000; ++v)
        ;
    res &= ~switches_pio->pdsr;
    return res;
}
