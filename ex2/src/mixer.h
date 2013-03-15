#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_NUM_VOICES    8
#define MAX_VOICE_LEN     256

#define BASE_NOTE         '0'
#define NOTE_SILENCE      '/'
#define NOTE_MAX          ((char) (BASE_NOTE+64))
#define BASE_FREQUENCY    110

typedef struct {
	char name[16];
	uint8_t num_voices;
	uint8_t length;
	uint32_t samples_per_note;
	char voices[MAX_NUM_VOICES][MAX_VOICE_LEN];
} struct melody;

typedef struct {
	const struct melody *mel;
	uint32_t _cur_sample;
	uint32_t _sample_rate;
	bool _should_loop;
} struct synthesis;

void synthesis_init(const struct melody* mel, bool should_loop, uint32_t sample_rate,
		struct synthesis *s);
bool synthesis_is_finished(const struct synthesis* s);
uint16_t synthesis_step(struct synthesis* s);


int synthesizer_synthesize(const struct melody* mel, int sample_rate,
                           short int *buffer, int buf_size);


#endif 
