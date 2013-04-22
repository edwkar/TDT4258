#include <assert.h>
#include <string.h>

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


#define WORLD_WIDTH                    FB_WIDTH
#define WORLD_HEIGHT                   FB_HEIGHT
#define LEFT_TANK_START_X              40
#define RIGHT_TANK_START_X             (WORLD_WIDTH - 40 - 40)

#define START_NUM_LIVES                2


static struct rate_keeper rk;

/* Game objects.
 */
static GameObject *GAME_OBJS[32];
static unsigned int num_game_objs = 0;

static Terrain *terrain = NULL;
static Tank *left_tank = NULL;
static Tank *right_tank = NULL;
static Projectile *projectile = NULL;
static bool left_tank_is_active = false;
static Text *text = NULL;


/* "Score table".
 */
static unsigned int num_lives_left = 0;
static unsigned int num_lives_right = 0;



struct state {
    struct state (*act)(uint32_t time_in_state);
};
#define SF(X) ((struct state){ X })

struct state state_init(uint32_t);
struct state state_prepare_round(uint32_t);
struct state state_player_act(uint32_t);
struct state state_projectile_move(uint32_t);
struct state state_projectile_explode(uint32_t);
struct state state_report_score(uint32_t);
struct state state_quit(uint32_t);
struct state state_report_winner(uint32_t);


struct state state_init(uint32_t __attribute__((unused)) time_in_state)
{

    /* Initialize the game objects.
     */
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

    /* Spin the intro movie at 18 (hopefully) FPS.
     */
    rk = rate_keeper_construct(18);
    screen_clear(PIXEL_BLACK);
    screen_redraw();
    audio_play(SC_RESOURCES_PATH "/audio/intro.raw");
    movie_play(SC_RESOURCES_PATH "/movies/intro.mpeg2", &rk);

    /* Now, keep the main action at 40 FPS.
     */
    rk = rate_keeper_construct(40);

    /* We'll duplicate this line from state_prepare_round,
     * otherwise we'll have an annoying 1-frame flash.
     */
    screen_set_opacity(0);

    num_lives_left = START_NUM_LIVES;
    num_lives_right = START_NUM_LIVES;

    return SF(state_prepare_round);
}

struct state state_prepare_round(uint32_t time_in_state)
{
    /* Darken the screen, so that we'll fade in.
     * and, let's wait here for a while.
     */
    if (time_in_state < 20) {
        screen_set_opacity(0);
        return SF(state_prepare_round);
    } else {
        FOR_EACH_GAME_OBJECT(GAME_OBJS, reset);
        left_tank_is_active = !left_tank_is_active;
        return SF(state_player_act);
    }
}

struct state state_player_act(uint32_t time_in_state)
{
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
        audio_play(SC_RESOURCES_PATH "/audio/fire.raw");

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
    assert(time_in_state < 300); // Sanity check.

    if (time_in_state == 0)
        printf("The projectile is airborne!\n");

    int32_t landing_pos[2];

    if (projectile->has_landed(projectile, landing_pos)) {
        projectile->reset(projectile);

        FOR_EACH_GAME_OBJECT(GAME_OBJS, apply_impact,
                             landing_pos[0], landing_pos[1]);

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
    }

    bool all_updates_done = !(
            left_tank->is_updating_from_impact(left_tank) ||
            right_tank->is_updating_from_impact(right_tank));

    if (all_updates_done && explode_leave_time == 0x424242) {
        audio_play(SC_RESOURCES_PATH "/audio/explode.raw");
        explode_leave_time = time_in_state + 30;
    }

    if (time_in_state < explode_leave_time)
        return SF(state_projectile_explode);

    screen_set_shaking(false);

    bool left_survives = left_tank->get_health(left_tank) != 0;
    bool right_survives = right_tank->get_health(right_tank) != 0;

    if (left_survives && right_survives) {
        left_tank_is_active = !left_tank_is_active;
        return SF(state_player_act);
    } else {
        if (!left_survives)
            num_lives_left--;
        if (!right_survives)
            num_lives_right--;

        bool both_alive = num_lives_left > 0 && num_lives_right > 0;

        return SF(both_alive ? state_report_score : state_report_winner);
    }
}

struct state state_report_score(uint32_t time_in_state)
{
    if (time_in_state == 100)
        return SF(state_prepare_round);

    if (time_in_state == 0) {
        char msg[256];

        size_t i = 0;

        for (size_t j = 0; j < num_lives_left; ++j)
            msg[i++] = 'X';
        msg[i++] = ' ';
        msg[i++] = ' ';
        for (size_t j = 0; j < num_lives_right; ++j)
            msg[i++] = 'X';
        msg[i] = '\0';

        text->add(text, TEXT_CENTER_HORIZONTAL, 180, msg, 2200);
    }

    return SF(state_report_score);
}

struct state state_report_winner(uint32_t time_in_state)
{
    if (time_in_state == 100)
        return SF(state_quit);

    if (time_in_state == 0) {
        char *msg = NULL;

        if (num_lives_left == 0 && num_lives_right == 0)
            msg = "You both suck!";
        else if (num_lives_right == 0)
            msg = "Left tank wins!";
        else if (num_lives_left == 0)
            msg = "Right tank wins!";
        else
            assert(false);

        text->add(text, TEXT_CENTER_HORIZONTAL, 180, msg, 2200);
    }

    return SF(state_report_winner);
}

struct state state_quit(uint32_t __attribute__((unused)) time_in_state)
{
    return SF(NULL /* I should never be dereferenced. */);
}

PROFILING_SETUP(main_update);
PROFILING_SETUP(main_render);

/* Hereth followeth the main game loop.
 */
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

        rate_keeper_tick(&rk);

        screen_redraw();
        screen_increment_opacity(15);

        bool is_new_state = next_state.act != cur_state.act;
        if (is_new_state) {
            time_in_state = 0;
            cur_state = next_state;
        }

        if (next_state.act == state_quit || input_key_is_down(input_6))
            break;
    }

    FOR_EACH_GAME_OBJECT(GAME_OBJS, destruct);
}
