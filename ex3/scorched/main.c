#include <stdbool.h>
#include <stdlib.h>

#include "audio.h"
#include "input.h"
#include "leds.h"
#include "screen.h"
#include "utils.h"
#include "shared_tests.h"

#include "scorched.h"


static void initialize_subsystems(void);
static void shutdown_subsystems(void);

int main(int argc, char **argv)
{
    initialize_subsystems();

    /* Either spin a test, if there is an argument,
     * or launch the game!
     */
    bool should_run_test = argc > 1;
    if (should_run_test)
        shared_tests_run(argv[1]);
    else
        scorched_run();

    shutdown_subsystems();

    return EXIT_SUCCESS;
}

static void initialize_subsystems(void)
{
    audio_init();
    input_init();
    leds_init();
    screen_init();
}

static void shutdown_subsystems(void)
{
    screen_cleanup();
    leds_cleanup();
    input_cleanup();
    audio_cleanup();
}
