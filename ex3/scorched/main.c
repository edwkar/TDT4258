#include <stdlib.h>

#include "audio.h"
#include "input.h"
#include "screen.h"
#include "utils.h"

#include "scorched.h"


static void initialize_subsystems(void);
static void shutdown_subsystems(void);

int main(void)
{
    initialize_subsystems();
    scorched_run();
    shutdown_subsystems();

    return EXIT_SUCCESS;
}

static void initialize_subsystems(void)
{
    input_init();
    audio_init();
    screen_init();
}

static void shutdown_subsystems(void)
{
    screen_cleanup();
    audio_cleanup();
}
