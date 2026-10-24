#include <stdio.h>
#include <string.h>
#include "input.h"
#include "race.h"
#include "track.h"
#include "trig.h"

Racer racers[NUM_CARS];
GameState game_state = STATE_TITLE;
uint16_t state_timer;
uint8_t finish_order;
uint8_t race_minutes, race_seconds, race_frames;
volatile bool autopilot;
volatile bool autopilot_drs;
bool race_log;
bool paused;

static uint8_t rescue_cooldown;
static int8_t player_steer;

/* Race statistics, printed to the console for tools/playtest.py. */
uint16_t stat_wall_hits[NUM_CARS];
uint16_t stat_car_hits[NUM_CARS];
uint8_t stat_reverses[NUM_CARS];
uint8_t stat_rescues[NUM_CARS];
uint16_t race_clock; /* frames since the start */
static uint16_t lap_start[NUM_CARS];

int16_t race_progress(const Racer *r)
{
    return (int16_t)r->lap * path_n + r->wp;
}

/* Squared distance to a racing line point, each axis capped at 127 px. */
static uint16_t dist2(int16_t px, int16_t py, uint8_t i)
{
    int16_t dx = path_x[i] - px;
    int16_t dy = path_y[i] - py;
    if (dx > 127) dx = 127;
    if (dx < -127) dx = -127;
    if (dy > 127) dy = 127;
    if (dy < -127) dy = -127;
    return (uint16_t)(dx * dx) + (uint16_t)(dy * dy);
}

/* Keep wp on the nearest point, moving along the loop only, so a car can't
 * skip ahead to a part of the track that happens to be close by. Crossing
 * point 0 counts a lap, either way. */
static void track_progress(Racer *r)
{
    int16_t px = PX(r->car.x);
    int16_t py = PX(r->car.y);
    uint8_t i = r->wp;
    uint16_t di = dist2(px, py, i);
    bool moved = false;
    for (uint8_t k = 0; k < 4; k++) {
        uint8_t n = path_next(i);
        uint16_t dn = dist2(px, py, n);
        if (dn > di)
            break;
        if (n == 0)
            r->lap++;
        i = n;
        di = dn;
        moved = true;
    }
    if (!moved) {
        uint8_t p = path_prev(i);
        if (dist2(px, py, p) < di) {
            if (i == 0)
                r->lap--;
            i = p;
        }
    }
    if (moved)
        r->idle = 0;
    else if (r->idle < 0xFFFF)
        r->idle++;
    r->wp = i;
}

/* Grid: two by two behind the line, the player third. */
static const uint8_t grid_slot[NUM_CARS] = {2, 0, 1, 3};

void race_setup(void)
{
    for (uint8_t i = 0; i < NUM_CARS; i++) {
        Racer *r = &racers[i];
        memset(r, 0, sizeof(*r));
        uint8_t slot = grid_slot[i];
        uint8_t idx = (uint8_t)(path_n - 2 - 3 * (slot >> 1));
        uint8_t h = path_heading[idx];
        int16_t off = (slot & 1) ? 10 : -10;
        if (off > (int16_t)path_room_r[idx]) off = path_room_r[idx];
        if (off < -(int16_t)path_room_l[idx]) off = -(int16_t)path_room_l[idx];
        int16_t x = path_x[idx] + ((off * COS(h)) >> 7);
        int16_t y = path_y[idx] - ((off * SIN(h)) >> 7);
        car_place(&r->car, x, y, h);
        r->car.accel = ACCEL_BASE;
        r->wp = idx;
        r->place = slot + 1;
        if (i != PLAYER)
            ai_init(r, i);
    }
    finish_order = 0;
    memset(stat_wall_hits, 0, sizeof(stat_wall_hits));
    memset(stat_car_hits, 0, sizeof(stat_car_hits));
    memset(stat_reverses, 0, sizeof(stat_reverses));
    memset(stat_rescues, 0, sizeof(stat_rescues));
    memset(lap_start, 0, sizeof(lap_start));
    race_clock = 0;
    race_minutes = race_seconds = race_frames = 0;
    rescue_cooldown = 0;
    player_steer = 0;
    paused = false;
}

void race_rescue(Racer *r)
{
    /* Back on the racing line, a little ahead, clear of other cars. */
    uint8_t i = path_add(r->wp, 1);
    for (uint8_t tries = 0; tries < 6; tries++) {
        bool clear = true;
        for (uint8_t j = 0; j < NUM_CARS; j++) {
            Racer *o = &racers[j];
            if (o == r)
                continue;
            int16_t dx = PX(o->car.x) - path_x[i];
            int16_t dy = PX(o->car.y) - path_y[i];
            if (dx > -16 && dx < 16 && dy > -16 && dy < 16)
                clear = false;
        }
        if (clear)
            break;
        i = path_add(i, 2);
    }
    /* Don't hand out a lap for a rescue across the line. */
    if (i < r->wp)
        i = r->wp;
    stat_rescues[r - racers]++;
    car_place(&r->car, path_x[i], path_y[i], path_heading[i]);
    r->wp = i;
    r->lane = r->lane_goal = 0;
    r->stuck = r->reverse = 0;
    r->idle = 0;
}

/* The player's steering eases in over a few frames, so a tap is a small
 * correction and holding it is full lock. Letting go, or reversing, is
 * immediate. */
#define STEER_RAMP 3

static int8_t steer_ramp(int8_t want)
{
    if (want == 0 || (want > 0) != (player_steer > 0))
        player_steer = 0;
    if (player_steer < want) {
        player_steer += STEER_RAMP;
        if (player_steer > want)
            player_steer = want;
    } else if (player_steer > want) {
        player_steer -= STEER_RAMP;
        if (player_steer < want)
            player_steer = want;
    }
    return player_steer;
}

static void player_drive(Racer *r)
{
    Car *c = &r->car;
    c->gas = input_gas();
    c->brake = input_brake() && !c->gas;
    c->steer = steer_ramp(input_steer());

    if (rescue_cooldown)
        rescue_cooldown--;
    if (is_action_just_pressed(ACTION_RESCUE) && !rescue_cooldown) {
        race_rescue(r);
        rescue_cooldown = 90;
    }
    if (is_action_just_pressed(ACTION_ALT_FIRE) && r->drs_charge >= DRS_FULL && !r->drs_timer) {
        r->drs_charge = 0;
        r->drs_timer = DRS_TIME;
    }
}

/* Is a ahead of b in the race? */
static bool ahead(const Racer *a, const Racer *b)
{
    if (a->finished || b->finished) {
        if (!b->finished)
            return true;
        if (!a->finished)
            return false;
        return a->place < b->place;
    }
    return race_progress(a) > race_progress(b);
}

static void update_places(void)
{
    for (uint8_t i = 0; i < NUM_CARS; i++) {
        Racer *r = &racers[i];
        if (r->finished)
            continue;
        uint8_t place = 1;
        for (uint8_t j = 0; j < NUM_CARS; j++)
            if (j != i && ahead(&racers[j], r))
                place++;
        r->place = place;
    }
}

static void update_drs(void)
{
    Racer *p = &racers[PLAYER];
    p->drs_in_range = false;
    for (uint8_t j = 1; j < NUM_CARS; j++) {
        if (racers[j].place + 1 == p->place && !racers[j].finished) {
            int16_t gap = race_progress(&racers[j]) - race_progress(p);
            p->drs_in_range = gap >= 0 && gap <= DRS_RANGE;
        }
    }
    if (p->drs_timer) {
        p->drs_timer--;
        p->car.accel = ACCEL_BASE + DRS_ACCEL;
        return;
    }
    p->car.accel = ACCEL_BASE;
    if (p->place > 1 && !p->finished && p->drs_charge < DRS_FULL) {
        /* Charges slowly anywhere behind the leader, fast when close
         * behind the car ahead: it's a tool for passing. */
        uint8_t add = p->drs_in_range ? 3 : (race_frames & 1);
        p->drs_charge = p->drs_charge + add > DRS_FULL ? DRS_FULL : p->drs_charge + add;
    }
}

/* Gentle rubber banding: at most about 10% more or less engine. */
static void update_rubber_band(void)
{
    Racer *p = &racers[PLAYER];
    int16_t pp = race_progress(p);
    for (uint8_t i = 1; i < NUM_CARS; i++) {
        Racer *r = &racers[i];
        int16_t adj = 0;
        if (!p->finished && !r->finished) {
            int16_t gap = pp - race_progress(r); /* + when the player leads */
            if (gap > 8)
                gap = 8;
            if (gap < -10)
                gap = -10;
            adj = gap * 3;
            /* No catching up on a player who is using DRS to pass. */
            if (p->drs_timer && adj > 0)
                adj = 0;
        }
        r->car.accel = (uint16_t)((int16_t)r->skill + adj);
    }
}

void race_step(void)
{
    for (uint8_t i = 0; i < NUM_CARS; i++) {
        Racer *r = &racers[i];
        if (i == PLAYER && !autopilot && !r->finished) {
            player_drive(r);
        } else {
            ai_drive(r);
            /* On autopilot, the player's car opens DRS when it can pass. */
            if (i == PLAYER && autopilot_drs && r->drs_charge >= DRS_FULL && r->drs_in_range) {
                r->drs_charge = 0;
                r->drs_timer = DRS_TIME;
            }
        }
    }

    update_drs();
    if ((race_frames & 7) == 0)
        update_rubber_band();

    for (uint8_t i = 0; i < NUM_CARS; i++) {
        Car *c = &racers[i].car;
        car_update(c);
        if (c->wall_hit == 7)
            stat_wall_hits[i]++;
    }

    /* The player gives a little less ground in a shove than the AI does. */
    for (uint8_t i = 0; i < NUM_CARS; i++)
        for (uint8_t j = i + 1; j < NUM_CARS; j++)
            car_contact(&racers[i].car, &racers[j].car, i == PLAYER ? 100 : 128);

    for (uint8_t i = 0; i < NUM_CARS; i++) {
        Racer *r = &racers[i];
        int8_t lap = r->lap;
        if (r->car.car_hit == 7)
            stat_car_hits[i]++;
        track_progress(r);
        if (r->lap > lap && lap > 0 && !r->finished) {
            if (race_log)
                printf("LAP car%u lap%d %u.%02us\n", i, lap, (race_clock - lap_start[i]) / 60,
                   (race_clock - lap_start[i]) % 60 * 5 / 3);
            lap_start[i] = race_clock;
        } else if (r->lap > lap && lap == 0) {
            lap_start[i] = race_clock;
        }
        if (!r->finished && r->lap > LAPS) {
            r->finished = true;
            r->place = ++finish_order;
            if (race_log)
                printf("FINISH car%u place%u %u.%02us walls%u bumps%u reverses%u rescues%u\n",
                   i, r->place, race_clock / 60, race_clock % 60 * 5 / 3,
                   stat_wall_hits[i], stat_car_hits[i], stat_reverses[i], stat_rescues[i]);
        }
    }
    race_clock++;
    update_places();

    if (!racers[PLAYER].finished) {
        if (++race_frames >= 60) {
            race_frames = 0;
            if (++race_seconds >= 60) {
                race_seconds = 0;
                if (race_minutes < 99)
                    race_minutes++;
            }
        }
    }
}
