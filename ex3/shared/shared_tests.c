#include <stdio.h>
#include <unistd.h>

#include "audio.h"
#include "input.h"
#include "leds.h"
#include "screen.h"
#include "shared_tests.h"
#include "utils.h"


static void __attribute__((noreturn)) test_input()
{
    for (;;) {
        putchar(input_key_is_down(input_0) ? '0' : ' ');
        putchar(input_key_is_down(input_1) ? '1' : ' ');
        putchar(input_key_is_down(input_2) ? '2' : ' ');
        putchar(input_key_is_down(input_3) ? '3' : ' ');
        putchar(input_key_is_down(input_4) ? '4' : ' ');
        putchar(input_key_is_down(input_5) ? '5' : ' ');
        putchar(input_key_is_down(input_6) ? '6' : ' ');
        putchar(input_key_is_down(input_7) ? '7' : ' ');
        putchar('\n');

        usleep(10 * 1000);
    }
}

static void __attribute__((noreturn)) test_leds()
{
    for (;;)
        for (uint8_t c = 0U; c < 255U; ++c) {
            leds_set(c);
            usleep(100 * 1000);
        }
}

static void test_screen()
{
    screen_draw_rect(  0, 0, 50, 200, PIXEL_RED);
    screen_draw_rect(100, 0, 50, 200, PIXEL_BLUE);
    screen_draw_rect(150, 0, 50, 200, PIXEL_GREEN);
    screen_redraw();
    usleep(2 * 1000 * 1000);

    for (int32_t stop = 1; stop <= 200; ++stop) {
        screen_clear(PIXEL_BLACK);
        for (int32_t i = 0; i < stop; ++i)
            screen_draw_rect(i, 0, 1, (uint32_t) (i+1), PIXEL_RED);
        screen_redraw();
    }

    screen_set_shaking(true);
    for (int32_t stop = 1; stop <= 200; ++stop) {
        screen_clear(PIXEL_BLACK);
        for (int32_t i = 0; i < stop; ++i)
            screen_draw_rect(i, 0, 1, (uint32_t) (i+1), PIXEL_RED);
        screen_redraw();
    }
    screen_set_shaking(false);

    screen_set_opacity(0);
    for (int32_t stop = 1; stop <= 200; ++stop) {
        screen_clear(PIXEL_BLACK);
        for (int32_t i = 0; i < stop; ++i)
            screen_draw_rect(i, 0, 1, (uint32_t) (i+1), PIXEL_RED);
        screen_increment_opacity(1);
        screen_redraw();
    }
}

static void test_audio()
{
    audio_play("shared/test.mp3.raw");
    sleep(1000);
}

void shared_tests_run(const char *test_name)
{
    if      (STREQ(test_name, "--test-input"))  test_input();
    else if (STREQ(test_name, "--test-leds"))   test_leds();
    else if (STREQ(test_name, "--test-screen")) test_screen();
    else if (STREQ(test_name, "--test-audio"))  test_audio();
    else {
        fprintf(stderr, "Unknown test '%s'.\n", test_name);
        exit(EXIT_FAILURE);
    }
}
