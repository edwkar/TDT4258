#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dac.h"
#include "synthesizer.h"
#include "utils.h"

#define AMP_PER_VOICE 4000
#define BASE_FREQ     110

enum synthesis_waveform {
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH
};
#define NUM_WAVEFORMS 3

struct synthesis {
    enum synthesis_waveform waveform;
    const struct melody *mel;
    uint32_t pos;
    uint32_t phases[MELODY_MAX_NUM_VOICES];
};


static bool __module_is_inited = false;

/* A lookup table mapping note numbers to waveform periods.
 */
static uint32_t NOTE_PERIODS[SYNTHESIZER_MAX_NOTE+1];

void synthesizer_init(uint32_t sample_rate) {
    assert(!__module_is_inited);

    for (int note = 0; note <= SYNTHESIZER_MAX_NOTE ; ++note) {
        // http://www.phy.mtu.edu/~suits/NoteFreqCalcs.html
        uint32_t freq = BASE_FREQ * pow(pow(2, 1/12.0), note);
        NOTE_PERIODS[note] = 2 * sample_rate / freq;
    }

    __module_is_inited = true;
}

/* struct synthesis object pool.
 */
static struct synthesis __object_pool[SYNTHESIS_OBJECT_POOL_SIZE];
static int __pool_num_created = 0;

struct synthesis * synthesis_construct(const struct melody *mel)
{
    assert(__pool_num_created < SYNTHESIS_OBJECT_POOL_SIZE);

    struct synthesis *s = &__object_pool[__pool_num_created++];
    s->waveform = WAVEFORM_SQUARE;
    s->mel = mel;
    s->pos = 0;

    return s;
}

void synthesis_next_waveform(struct synthesis *s) {
    s->waveform = (s->waveform+1) % NUM_WAVEFORMS;
}

static ALWAYS_INLINE(
void write_note(int32_t note,
                enum synthesis_waveform waveform,
                uint32_t * phase,
                uint32_t num_to_write,
                int16_t buf[static num_to_write]));

static ALWAYS_INLINE(
uint16_t note_id_at(const int32_t *voice, int32_t pos));

uint32_t synthesis_advance(struct synthesis *s,
                           uint32_t tot_to_write,
                           int16_t res[static tot_to_write])
{
    assert(__module_is_inited);

    const struct melody *mel = s->mel;
    assert(s->pos != mel->len);

    for (int v_id = 0; v_id < mel->num_voices; ++v_id) {
        const int32_t *v = mel->voices[v_id];
        uint32_t v_pos = s->pos; // Position in voice.
        uint32_t res_pos = 0;    // Position in result buffer.

        while (res_pos < tot_to_write && v_pos < mel->len) {
            uint32_t note_id = note_id_at(v, v_pos);
            int32_t note = v[2*note_id+1];

            uint32_t note_samples_rem = v[2*note_id+2] - v_pos;

            uint32_t num_to_write = MIN(tot_to_write - res_pos,
                                        note_samples_rem);
            assert(num_to_write != 0);

            write_note(note, s->waveform, &s->phases[v_id],
                       num_to_write, res + res_pos);

            res_pos += num_to_write;
            v_pos += num_to_write;
        }
    }

    uint32_t advancement = MIN(tot_to_write, mel->len - s->pos);

    s->pos += advancement;
    assert(s->pos <= mel->len);

    return advancement;
}

void synthesis_rewind(struct synthesis *s)
{
    s->pos = 0;
}

bool synthesis_is_at_end(const struct synthesis *s)
{
    return s->pos == s->mel->len;
}

static void write_note(int32_t note,
                       enum synthesis_waveform waveform,
                       uint32_t * phase,
                       uint32_t num_to_write,
                       int16_t buf[static num_to_write])
{
    if (note == MELODY_NOTE_SILENCE)
        return;

    uint32_t period = NOTE_PERIODS[note];
    uint32_t half_period = period / 2;

    for (int16_t *p = buf; p != buf+num_to_write; ++p, ++*phase) {
        switch (waveform) {
        case WAVEFORM_SQUARE:
            if (*phase < half_period)
                *p += AMP_PER_VOICE;
            break;

        case WAVEFORM_TRIANGLE:
            if (*phase < half_period)
                *p += *phase * AMP_PER_VOICE / half_period;
            else
                *p += (period-*phase) * AMP_PER_VOICE / half_period;
            break;

        case WAVEFORM_SAWTOOTH:
            *p += *phase * AMP_PER_VOICE / period;
            break;

        default:
            PANIC("Unrecognized waveform type.");
        }

        if (*phase >= period)
            *phase = 0;
    }
}

static uint16_t note_id_at(const int32_t *voice, int32_t pos)
{
    for (int i = 0; ; ++i)
        if (voice[2*i] <= pos && pos < voice[2*i+2])
            return i;
}
