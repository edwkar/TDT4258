#include <assert.h>
#include <avr32/ap7000.h>
#include <avr32/ap7000_intc.h>
#include <avr32/dac_101.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/interrupts.h>
#include "dac.h"
#include "leds.h"
#include "melodies.h"
#include "switches.h"
#include "synthesizer.h"
#include "controller.h"


#define NUM_SYNTHS 4

#define LOOP_SWITCH               SWITCH_FROM_RIGHT(1)
#define LOOP_SWITCH_LED           LED_FROM_RIGHT(1)

#define WAVEFORM_NEXT_SWITCH      SWITCH_FROM_RIGHT(2)
#define WAVEFORM_NEXT_SWITCH_LED  LED_FROM_RIGHT(2)

#define DAC_WAIT_LED              LED_FROM_RIGHT(0)

#define SWITCH_FOR_TRACK(i)       SWITCH_FROM_LEFT(i)
#define LED_FOR_TRACK(i)          LED_FROM_LEFT(i)


/* Melodies loaded at start.
 */
static const char mel_names[NUM_SYNTHS][16] = {
    "got_loop", // Melody for switch 7
    "prelude2", //                ...6
    "dungeon",  //                ...5
    "blipp"     //                ...4
};


/* Synthesis data structures.
 */
static struct controller *cont;
static struct synthesis *synths[NUM_SYNTHS];
static int16_t render_buf[DAC_BUF_SIZE];


static void initialize_modules(void);
static void setup_melody_controller(void);
static void run_main_loop(void);
static void update_track_leds(void);
static void update_from_switches(uint32_t switch_state);

int main(void)
{
    /* Even though we do not *enable* interrupts at once,
     * we must configure the interrupt controller, to
     * be able to register interrupts.
     */
    set_interrupts_base((void *) AVR32_INTC_ADDRESS);

    /* Now, initialize all sub-systems.
     */
    initialize_modules();
    setup_melody_controller();

    /* All sub-systems are ready for control;
     * register the switch listener, to enable interactivity.
     */
    switches_use_listener(update_from_switches);

    /* Enable interrupts.
     */
    init_interrupts();

    /* Finally, enter the main loop.
     */
    run_main_loop();

    return EXIT_SUCCESS;
}

static void initialize_modules(void) {
    switches_init();
    leds_init();
    dac_init();
    synthesizer_init(DAC_SAMPLE_RATE);
}

static void setup_melody_controller(void) {
    cont = controller_construct();

    for (int i = 0; i < NUM_SYNTHS; ++i) {
        synths[i] = synthesis_construct(melodies_get(mel_names[i]));
        controller_add_track(cont, synths[i]);
    }
}

static void run_main_loop(void) {
    for (;;) {
        /* The sound-generating procedures do not overwrite their output
         * buffers, they only add deltas. Thus, the buffer should be
         * cleared before each advancement.
         */
        memset(render_buf, 0, sizeof render_buf);

        /* Run synthesis on all live tracks!
         */
        controller_advance(cont, DAC_BUF_SIZE, render_buf);

        update_track_leds();

        /* Wait till the DAC module can accept data.
         * Indicate the waiting by lighting an indicator LED.
         */
        leds_light(DAC_WAIT_LED);
        while (!dac_may_write())
            ;
        leds_turn_off(DAC_WAIT_LED);

        dac_write_buffer(render_buf);
    }
}

static void update_track_leds(void) {
    static int num_calls = 0;

    for (uint32_t i = 0; i < NUM_SYNTHS; ++i) {
        bool is_active = controller_track_is_active(cont, i);

        bool is_looping = controller_track_is_looping(cont, i);
        bool should_light_if_looping = (num_calls%16) < 8;

        bool should_light = is_active && (!is_looping||should_light_if_looping);
        leds_lighted_if(LED_FOR_TRACK(i), should_light);
    }

    num_calls++;
}

static void update_from_switches(uint32_t switch_state) {
    bool should_loop = SWITCH_IS_PRESSED(switch_state, LOOP_SWITCH);
    bool should_switch_waveform = SWITCH_IS_PRESSED(switch_state,
                                                    WAVEFORM_NEXT_SWITCH);

    /* Light the "meta"-switches whenever they are pressed.
     */
    leds_lighted_if(LOOP_SWITCH_LED, should_loop);
    leds_lighted_if(WAVEFORM_NEXT_SWITCH_LED, should_switch_waveform);

    for (int i = 0; i < NUM_SYNTHS; ++i) {
        bool switch_is_pressed = SWITCH_IS_PRESSED(switch_state,
                                                   SWITCH_FOR_TRACK(i));
        if (!switch_is_pressed)
            continue;

        if (should_switch_waveform)
            synthesis_next_waveform(synths[i]);
        else if (!controller_track_is_active(cont, i))
            controller_start_track(cont, i, should_loop);
        else
            controller_stop_track(cont, i);
    }
}
