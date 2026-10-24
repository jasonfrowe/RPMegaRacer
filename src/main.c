#include <rp6502.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gfx.h"
#include "hud.h"
#include "input.h"
#include "opl.h"
#include "race.h"
#include "sound.h"
#include "track.h"

/* For tools/playtest.py: frames the main loop missed, and frames run. */
uint8_t lag_frames;
uint16_t frame_count;
/* For tools/playtest.py: poke a track number here on the title screen. */
volatile uint8_t debug_track;

/* Test mode, for the headless emulator:
 *   rp6502-emu RPMegaRacer.rp6502 --headless --phi2 0 -- race <track> [drs]
 * The AI drives every car through one race, the console gets each lap and
 * finish, and the program exits when the player's car is home. */
static bool test_mode;

/* Room for argv; llvm-mos skips the arguments without it. */
void *__argv_mem(size_t size)
{
    static char args[256];
    return size <= sizeof(args) ? args : NULL;
}

#define COUNTDOWN_FRAMES 240 /* 3, 2, 1 and GO a second each */

static void enter_title(void)
{
    stop_engine_sound();
    hud_clear();
    race_setup();
    gfx_title_show();
    game_state = STATE_TITLE;
}

static void start_race(void)
{
    gfx_title_hide();
    hud_clear();
    race_setup();
    init_opl2_engine_sound();
    game_state = STATE_COUNTDOWN;
    state_timer = COUNTDOWN_FRAMES;
}

static void engine_sound(void)
{
    int16_t sp = racers[PLAYER].car.speed;
    update_engine_sound((uint16_t)(sp < 0 ? -sp : sp));
}

static void race_frame(void)
{
    if (is_action_just_pressed(ACTION_PAUSE)) {
        paused = !paused;
        if (paused)
            hud_center(13, " PAUSED ", HUD_COL_WHITE);
        else
            hud_clear_row(13);
    }
    if (paused)
        return;

    race_step();
    hud_race();
    engine_sound();

    /* The GO! stays up for the first second of the race. */
    if (state_timer && --state_timer == 0)
        hud_clear_row(13);

    if (racers[PLAYER].finished) {
        stop_engine_sound();
        game_state = STATE_FINISHED;
        state_timer = 180;
    }
}

static void finished_frame(void)
{
    /* The race goes on behind the results; the AI drives the player home. */
    race_step();
    hud_results();
    if (state_timer) {
        state_timer--;
        return;
    }
    if (test_mode) {
        printf("LAG %u frames over %u\n", lag_frames, frame_count);
        exit(0);
    }
    hud_center(18, (RIA.vsync & 16) ? " PRESS START " : "             ", HUD_COL_WHITE);
    if (is_action_just_pressed(ACTION_PAUSE) || is_action_just_pressed(ACTION_FIRE)) {
        if (racers[PLAYER].place == 1) {
            track_load(current_track < NUM_TRACKS ? current_track + 1 : 1);
            start_race();
        } else {
            track_load(1);
            enter_title();
        }
    }
}

int main(int argc, char **argv)
{
    gfx_init();
    input_init();

    OPL_Config(1, OPL_ADDR);
    opl_init();
    music_init(MUSIC_FILENAME);

    track_load(1);
    enter_title();
    if (argc >= 3 && strcmp(argv[1], "race") == 0) {
        test_mode = true;
        autopilot = true;
        race_log = true;
        autopilot_drs = argc >= 4 && strcmp(argv[3], "drs") == 0;
        track_load((uint8_t)atoi(argv[2]));
        start_race();
    }

    uint8_t vs = RIA.vsync;
    while (1) {
        uint8_t now;
        while ((now = RIA.vsync) == vs)
            ;
        if ((uint8_t)(now - vs) > 1)
            lag_frames++;
        vs = now;
        frame_count++;

        gfx_commit();
        update_music();
        input_update();

        switch (game_state) {
        case STATE_TITLE:
            if (debug_track) {
                track_load(debug_track);
                race_setup();
                debug_track = 0;
            }
            hud_title();
            if (is_action_just_pressed(ACTION_FIRE) || is_action_just_pressed(ACTION_PAUSE))
                start_race();
            break;
        case STATE_COUNTDOWN:
            hud_race();
            hud_countdown(state_timer);
            if (--state_timer == 60)
                game_state = STATE_RACING;
            break;
        case STATE_RACING:
            race_frame();
            break;
        case STATE_FINISHED:
            finished_frame();
            break;
        }

        gfx_prepare();
    }
}
