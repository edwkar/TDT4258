#ifndef __SYNTHESIZER_H
#define __SYNTHESIZER_H

#include <stdbool.h>
#include <stdint.h>

#define SYNTHESIZER_MAX_NOTE        128 
#define SYNTHESIS_OBJECT_POOL_SIZE  16

#define MELODY_MAX_NUM_VOICES       8
#define MELODY_MAX_VOICE_LEN        2048
#define MELODY_NOTE_SILENCE         -1

struct melody {
    char name[24];
    uint8_t num_voices;
    uint32_t len;
    int32_t voices[MELODY_MAX_NUM_VOICES][MELODY_MAX_VOICE_LEN];
};

struct synthesis;

void synthesizer_init(uint32_t sample_rate);

struct synthesis * synthesis_construct(const struct melody *mel);
uint32_t synthesis_advance(struct synthesis *s,
                           uint32_t tot_to_write,
                           int16_t res[static tot_to_write]);
void synthesis_rewind(struct synthesis *s);
bool synthesis_is_at_end(const struct synthesis *s);
void synthesis_next_waveform(struct synthesis *s);

#endif 
