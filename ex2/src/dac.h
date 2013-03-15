#ifndef __DAC_H
#define __DAC_H

#include <stdbool.h>
#include <stdint.h>

#define DAC_SAMPLE_RATE     46875
#define DAC_BUF_SIZE        2048

void dac_init(void);
bool dac_may_write(void);
void dac_write_buffer(const int16_t buf[static DAC_BUF_SIZE]);

#endif
