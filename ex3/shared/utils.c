#include "audio.h"
#include "input.h"
#include "screen.h"
#include "utils.h"

void * malloc_or_die(size_t s) {
    void *v = malloc(s);
    if (!v)
        die_hard("malloc");
    return v;
};

void initialize_subsystems(void) {
    input_init();
    audio_init();
    screen_init();
}

void cleanup_subsystems(void) {
    screen_cleanup();
    audio_cleanup();
}
