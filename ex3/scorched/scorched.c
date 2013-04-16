#include <assert.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "audio.h"
#include "input.h"
#include "movie.h"
#include "profiling.h"
#include "ratekeeper.h"
#include "scorched.h"
#include "screen.h"
#include "utils.h"

#include "Projectile.h"
#include "Tank.h"
#include "Terrain.h"
#include "Text.h"


#define WORLD_WIDTH FB_WIDTH
#define WORLD_HEIGHT FB_HEIGHT
#define LEFT_TANK_START_X 40
#define RIGHT_TANK_START_X (WORLD_WIDTH - 40 - 40)

static struct rate_keeper rt_keeper;

static Terrain *terrain = NULL;
static Tank *left_tank = NULL;
static Tank *right_tank = NULL;
static Projectile *projectile = NULL;
static bool left_tank_is_active = false;

static Text *text = NULL;

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

struct state state_init(uint32_t __attribute__((unused)) time_in_state)
{
    assert(num_game_objs == 0);

    terrain = Terrain_construct(WORLD_WIDTH, WORLD_HEIGHT);
    GAME_OBJS[num_game_objs++] = (GameObject*) terrain;

    left_tank = Tank_construct(terrain, LEFT_TANK_START_X, true);
    GAME_OBJS[num_game_objs++] = (GameObject*) left_tank;

    right_tank = Tank_construct(terrain, RIGHT_TANK_START_X, false);
    GAME_OBJS[num_game_objs++] = (GameObject*) right_tank;

    projectile = Projectile_construct(terrain);
    GAME_OBJS[num_game_objs++] = (GameObject*) projectile;

    text = Text_construct();
    GAME_OBJS[num_game_objs++] = (GameObject*) text;

    rt_keeper = rate_keeper_construct(18);
    movie_play(SC_RESOURCES_PATH "/movies/intro.mpeg2", &rt_keeper);
    rt_keeper = rate_keeper_construct(40);

    screen_set_opacity(0);

    return SF(state_reset);
}

struct state state_reset(uint32_t time_in_state)
{
    if (time_in_state == 0)
        screen_set_opacity(0);

    FOR_EACH_GAME_OBJECT(GAME_OBJS, reset);

    left_tank_is_active = !left_tank_is_active;
    return SF(state_player_act);
}

struct state state_player_act(uint32_t time_in_state)
{
    assert(time_in_state < 10000);

    Tank *t = left_tank_is_active ? left_tank : right_tank;
    if (time_in_state == 0) {
        char msg[128];
        snprintf(msg, sizeof msg, "%s tank is up!\n",
                                  left_tank_is_active ? "Left" : "Right");
        text->add(text, TEXT_CENTER_HORIZONTAL, 180, msg, 2200);
    }

    int32_t x, y;
    uint32_t charge, angle;

    if (t->has_released(t, &x, &y, &charge, &angle)) {
        float charge_scale = 0.2F;
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
}

struct state state_projectile_move(uint32_t time_in_state)
{
    assert(time_in_state < 300);

    if (time_in_state == 0)
        printf("The projectile is airborne!\n");

    int32_t landing_pos[2];

    if (projectile->has_landed(projectile, landing_pos)) {
        projectile->reset(projectile);
        FOR_EACH_GAME_OBJECT(GAME_OBJS, apply_impact, landing_pos[0], landing_pos[1]);
        return SF(state_projectile_explode);
    } else
        return SF(state_projectile_move);
}

static uint32_t explode_leave_time = 0;
struct state state_projectile_explode(uint32_t time_in_state)
{
    if (time_in_state == 0) {
        screen_set_shaking(true);
        explode_leave_time = 0x424242;
        printf("The projectile explodes!\n");
    }

    bool are_all_updates_done = !(
            left_tank->is_updating_from_impact(left_tank) ||
            right_tank->is_updating_from_impact(right_tank));

    if (are_all_updates_done && explode_leave_time == 0x424242)
        explode_leave_time = time_in_state + 30;

    if (time_in_state == explode_leave_time) {
        screen_set_shaking(false);

        bool left_lives = left_tank->get_health(left_tank) != 0;
        bool right_lives = right_tank->get_health(right_tank) != 0;

        char msg[128];
        snprintf(msg, sizeof msg, "%c %c\n", 'A'+left_lives, 'A'+right_lives);
        text->add(text, TEXT_CENTER_HORIZONTAL, 150, msg, 3500);

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

PROFILING_SETUP(main_update);
PROFILING_SETUP(main_render);

void scorched_run(void)
{
    struct state cur_state = { state_init };
    uint32_t time_in_state = 0;

    for (;;) {
        struct state next_state = cur_state.act(time_in_state++);

        PROFILING_ENTER(main_update);
        FOR_EACH_GAME_OBJECT(GAME_OBJS, update);
        PROFILING_EXIT(main_update);

        PROFILING_ENTER(main_render);
        FOR_EACH_GAME_OBJECT(GAME_OBJS, render);
        PROFILING_EXIT(main_render);

        rate_keeper_tick(&rt_keeper);

        screen_increment_opacity(15);
        screen_redraw();

        bool is_new_state = next_state.act != cur_state.act;
        if (is_new_state) {
            time_in_state = 0;
            cur_state = next_state;
        }

        if (input_key_is_down(input_6))
            break;
    }

    FOR_EACH_GAME_OBJECT(GAME_OBJS, destruct);
}
