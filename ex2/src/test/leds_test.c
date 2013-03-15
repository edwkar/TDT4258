#include "leds.h"
#include "utils.h"

int main(void) {
    leds_init();

    for (int i = 0; i < 8; ++i)
        assert(LED_FROM_LEFT(i) == LED_FROM_RIGHT(7-i));

    {
        for (int i = 0; i < 4; ++i) {
            leds_light_all();
            tiny_spinlock_wait();

            leds_turn_off_all();
            tiny_spinlock_wait();
        }
    }

    {
        for (int i = 0; i < LEDS_CNT; ++i) {
            leds_light(LED_FROM_LEFT(i));
            tiny_spinlock_wait();
            leds_turn_off_all();
        }

        for (int i = 0; i < LEDS_CNT; ++i) {
            leds_light(LED_FROM_RIGHT(i));
            tiny_spinlock_wait();
            leds_turn_off_all();
        }
    }

    {
        for (int i = 0; i < LEDS_CNT; ++i) {
            leds_light_all();
            leds_turn_off(LED_FROM_LEFT(i));
            tiny_spinlock_wait();
        }
    }

    {
        leds_turn_off_all();
        for (int i = 0; i < LEDS_CNT; ++i) {
            leds_lighted_if(LED_FROM_LEFT(i), i%2 == 0);
            tiny_spinlock_wait();
        }

        leds_turn_off_all();
        for (int i = 0; i < LEDS_CNT; ++i) {
            leds_lighted_if(LED_FROM_LEFT(i), i%2 != 0);
            tiny_spinlock_wait();
        }
    }

    return 0;
}
