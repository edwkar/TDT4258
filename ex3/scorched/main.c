#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/soundcard.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "audio.h"
#include "input.h"
#include "screen.h"
#include "utils.h"
#include "Tank.h"
#include "Terrain.h"
#include "Projectile.h"

static void run_game(void);

int main(void)
{
    initialize_subsystems();
    printf("Subsystems initialized...\n");
    run_game();
    cleanup_subsystems();

    return EXIT_SUCCESS;
}



#define FOR_EACH(XS, METH_NAME, ...) \
    for (int __i = 0, __n = sizeof XS / sizeof XS[0]; __i < __n; ++__i) \
        if (XS[__i] && XS[__i]->METH_NAME != NULL) \
            XS[__i]->METH_NAME(XS[__i], ##__VA_ARGS__);

#define WORLD_WIDTH FB_WIDTH
#define WORLD_HEIGHT FB_HEIGHT
#define LEFT_TANK_START_X 40
#define RIGHT_TANK_START_X (WORLD_WIDTH - 40 - 40)


static Terrain *terrain = NULL;
static Tank *left_tank = NULL;
static Tank *right_tank = NULL;
static Projectile *projectile = NULL;
static bool left_tank_is_active = true;

static unsigned int num_wins_left = 0;
static unsigned int num_wins_right = 0;

static GameObject *GAME_OBJS[32];
static unsigned int num_game_objs = 0;

struct state {
    struct state (*act)(uint32_t);
};
#define SF(X) ((struct state){ X })

struct state state_init(uint32_t time_in_state);
struct state state_reset(uint32_t time_in_state);
struct state state_player_act(uint32_t time_in_state);
struct state state_projectile_move(uint32_t time_in_state);
struct state state_projectile_explode(uint32_t time_in_state);

struct state state_init(uint32_t time_in_state) {
    if (time_in_state == 12313213) exit(-1);

    assert(num_game_objs == 0);

    terrain = Terrain_construct(WORLD_WIDTH, WORLD_HEIGHT);
    GAME_OBJS[num_game_objs++] = (GameObject*) terrain;

    left_tank = Tank_construct(terrain, LEFT_TANK_START_X, true);
    GAME_OBJS[num_game_objs++] = (GameObject*) left_tank;

    right_tank = Tank_construct(terrain, RIGHT_TANK_START_X, false);
    GAME_OBJS[num_game_objs++] = (GameObject*) right_tank;

    projectile = Projectile_construct(terrain);
    GAME_OBJS[num_game_objs++] = (GameObject*) projectile;

    return SF(state_reset);
}

struct state state_reset(uint32_t time_in_state) {
    if (time_in_state == 12313213) exit(-1);

    printf("Resetting game.\n");

    FOR_EACH(GAME_OBJS, reset);

    left_tank_is_active = !left_tank_is_active;
    return SF(state_player_act);
};

struct state state_player_act(uint32_t time_in_state) {
    assert(time_in_state < 10000);

    Tank *t = left_tank_is_active ? left_tank : right_tank;
    if (time_in_state == 0)
        printf("%s tank to act!\n", left_tank_is_active ? "Left" : "Right");

    uint16_t x, y, charge;
    int16_t angle;
    if (t->has_released(t, &x, &y, &charge, &angle)) {
        double charge_scale = 0.2;
        t->clear_release(t);
        projectile->fire_from(projectile, x, y, angle, charge_scale * charge);
        return SF(state_projectile_move);
    } else {
        TankInput input = {
            .move_left     = input_key_is_down(input_0),
            .turret_left   = input_key_is_down(input_2),
            .turret_right  = input_key_is_down(input_1),
            .move_right    = input_key_is_down(input_3),
            .turret_charge = input_key_is_down(input_4)
        };
        t->set_input(t, input);

        return SF(state_player_act);
    }
};

struct state state_projectile_move(uint32_t time_in_state) {
    assert(time_in_state < 300);

    if (time_in_state == 0)
        printf("The projectile is airborne!\n");

    uint16_t landing_pos[2];

    if (projectile->has_landed(projectile, landing_pos)) {
        projectile->reset(projectile);
        FOR_EACH(GAME_OBJS, apply_impact, landing_pos[0], landing_pos[1]);
        return SF(state_projectile_explode);
    } else
        return SF(state_projectile_move);

    if (time_in_state == 12313213) exit(-1);
}

struct state state_projectile_explode(uint32_t time_in_state) {
    if (time_in_state == 0)
        printf("The projectile explodes!\n");

    bool are_all_updates_done = !(
            left_tank->is_updating_from_impact(left_tank) ||
            right_tank->is_updating_from_impact(right_tank));

    if (are_all_updates_done) {
        bool left_lives = left_tank->get_health(left_tank) != 0;
        bool right_lives = right_tank->get_health(right_tank) != 0;
        printf("%d %d\n", left_lives, right_lives);

        left_tank_is_active = !left_tank_is_active;

        if (left_lives && right_lives)
            return SF(state_player_act);
        else if (left_lives) {
            num_wins_left++;
            return SF(state_reset);
        } else if (right_lives) {
            num_wins_right++;
            return SF(state_reset);
        } else
            return SF(state_reset);
    } else
        return SF(state_projectile_explode);
}

static void run_game(void)
{
    struct state cur_state = { state_init };
    uint32_t time_in_state = 0;

    for (;;) {
        struct state next_state = cur_state.act(time_in_state++);

        FOR_EACH(GAME_OBJS, update_physics);
        FOR_EACH(GAME_OBJS, render);
        screen_redraw();

#ifdef HOST_BUILD
        usleep(10 * 1000);
#endif
        bool is_new_state = next_state.act != cur_state.act;
        if (is_new_state) {
            time_in_state = 0;
            cur_state = next_state;
        }

        if (input_key_is_down(input_6))
            break;
    }

    FOR_EACH(GAME_OBJS, destruct);
}
