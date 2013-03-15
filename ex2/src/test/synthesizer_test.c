#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "synthesizer.h"
#include "melodies.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [song name]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    synthesizer_init(48000);

    char *mel_name = argv[1];
    const struct melody *mel = melodies_get(mel_name);
    struct synthesis *s = synthesis_construct(mel);


    for (int i = 0; i < 2000; ++i) {
        uint32_t sz = 1 + (rand()%4 ? rand()%2048 : rand()%10);
        fprintf(stderr, "%d: %u\n", i, sz);
        int16_t buf[sz];

        memset(buf, 0, sizeof buf);
        int num_written = 0;
        while (num_written < sz) {
            if (synthesis_is_at_end(s)) {
                synthesis_next_waveform(s);
                synthesis_rewind(s);
            }
            int nw = synthesis_advance(s, sz-num_written, buf+num_written);
            num_written += nw;
        }

        for (uint32_t i = 0; i < sz; ++i) {
            putchar((char) buf[i]);
            putchar((char) (buf[i] >> 8));
        }
    }

    return 0;
}
