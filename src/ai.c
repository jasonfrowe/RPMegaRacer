#include "race.h"
#include "track.h"
#include "trig.h"

extern uint8_t stat_reverses[NUM_CARS];

/* Each AI drives its own lane at its own pace. */
static const int8_t lane_home[NUM_CARS] = {0, -5, 0, 5};
static const uint16_t skill[NUM_CARS] = {ACCEL_BASE, ACCEL_BASE - 8, ACCEL_BASE, ACCEL_BASE + 6};
static const int8_t pace[NUM_CARS] = {0, -10, 0, 8};

#define PASS_GAP 14     /* px beside a car to pass it */
#define LOOK_AHEAD 6    /* racing line points ahead that a car looks for traffic */

void ai_init(Racer *r, uint8_t n)
{
    r->lane_home = lane_home[n];
    r->lane = r->lane_goal = r->lane_home;
    r->skill = skill[n];
    r->pace = pace[n];
    r->car.accel = r->skill;
}

/* px a car is right of the racing line at point i. */
static int8_t lateral(const Car *c, uint8_t i)
{
    uint8_t h = path_heading[i];
    int16_t dx = PX(c->x) - path_x[i];
    int16_t dy = PX(c->y) - path_y[i];
    if (dx > 100) dx = 100;
    if (dx < -100) dx = -100;
    if (dy > 100) dy = 100;
    if (dy < -100) dy = -100;
    int16_t l = (dx * COS(h) - dy * SIN(h)) >> 7;
    return (int8_t)l;
}

static int8_t clamp_lane(int16_t lane, uint8_t i)
{
    int16_t hi = (int16_t)path_room_r[i] - 2;
    int16_t lo = 2 - (int16_t)path_room_l[i];
    if (hi < 0) hi = 0;
    if (lo > 0) lo = 0;
    if (lane > hi) lane = hi;
    if (lane < lo) lane = lo;
    return (int8_t)lane;
}

static bool lane_ok(int16_t lane, uint8_t i)
{
    return lane >= 2 - (int16_t)path_room_l[i] && lane <= (int16_t)path_room_r[i] - 2;
}

/* Look for cars just ahead or alongside, and pick a lane round them.
 * Returns the speed to hold if there's no way by, else 0x7FFF. */
static int16_t traffic(Racer *r)
{
    int16_t me = race_progress(r);
    int8_t my_l = lateral(&r->car, r->wp);
    int16_t hold = 0x7FFF;
    int16_t goal = r->lane_home;
    for (uint8_t j = 0; j < NUM_CARS; j++) {
        Racer *o = &racers[j];
        if (o == r)
            continue;
        int16_t gap = race_progress(o) - me;
        if (gap < 0 || gap > LOOK_AHEAD)
            continue; /* a car level or behind gives way to us */
        int8_t ol = lateral(&o->car, o->wp);
        int16_t d = (int16_t)my_l - ol;
        if (d >= PASS_GAP || d <= -PASS_GAP)
            continue;
        if (gap <= 1) {
            /* Alongside: keep our side of it, or tuck in behind. */
            int16_t keep = d >= 0 ? ol + PASS_GAP : ol - PASS_GAP;
            if (lane_ok(keep, r->wp))
                goal = keep;
            else if (o->car.speed - 32 < hold)
                hold = o->car.speed - 32;
            continue;
        }
        /* Ahead: only matters if we're catching it. */
        if (o->car.speed > r->car.speed + 64)
            continue;
        /* Go round on the side we're already on, if there's room. */
        int16_t left = ol - PASS_GAP;
        int16_t right = ol + PASS_GAP;
        bool left_ok = lane_ok(left, o->wp);
        bool right_ok = lane_ok(right, o->wp);
        if (left_ok && (d < 0 || !right_ok))
            goal = left;
        else if (right_ok)
            goal = right;
        else if (o->car.speed - 16 < hold)
            hold = o->car.speed - 16; /* boxed in: follow, don't ram */
    }
    r->lane_goal = clamp_lane(goal, r->wp);
    return hold;
}

void ai_drive(Racer *r)
{
    Car *c = &r->car;
    int16_t px = PX(c->x);
    int16_t py = PX(c->y);
    int16_t sp = c->speed;

    int16_t hold = traffic(r);
    if (r->lane < r->lane_goal)
        r->lane++;
    else if (r->lane > r->lane_goal)
        r->lane--;

    /* Aim at a point on our lane further ahead the faster we go, but close
     * in for a tight corner, or we'd cut across its inside wall. */
    uint8_t look = 2 + (uint8_t)((sp > 0 ? sp : 0) >> 8);
    uint8_t near = path_add(r->wp, 2);
    if (path_speed[near] < 100 || path_speed[path_next(near)] < 100)
        look = 2;
    uint8_t t = path_add(r->wp, look);
    int8_t lane = clamp_lane(r->lane, t);
    uint8_t h = path_heading[t];
    int16_t tx = path_x[t] + ((lane * COS(h)) >> 7);
    int16_t ty = path_y[t] - ((lane * SIN(h)) >> 7);
    uint8_t want = heading_to(tx - px, ty - py);
    int8_t err = (int8_t)(want - (uint8_t)(c->heading >> 8));

    int16_t steer = err * 2;
    if (steer > 16) steer = 16;
    if (steer < -16) steer = -16;
    c->steer = (int8_t)steer;

    /* Speed: the slowest hint between here and the aim point. */
    uint8_t hint = 255;
    for (uint8_t k = 0, i = r->wp; k <= look; k++, i = path_next(i))
        if (path_speed[i] < hint)
            hint = path_speed[i];
    int16_t target = (int16_t)hint * 4;
    target += (int16_t)((target >> 4) * r->pace) >> 4;
    if (r->finished)
        target -= target >> 2; /* cool down lap */
    if (target > hold)
        target = hold;

    int8_t aerr = err < 0 ? -err : err;
    /* Ease off for a sharp turn, unless we're too slow to turn well. */
    c->gas = sp < target && (aerr < 40 || sp < 160);
    c->brake = sp > target + 64;

    /* Unstick: if we're trying to go and not moving, back out while
     * turning toward the line, then try again. */
    if (r->reverse) {
        r->reverse--;
        c->gas = false;
        c->brake = true;
        if (r->reverse == 0)
            r->stuck = 0;
    } else if (c->gas && sp < 48 && sp > -48) {
        if (++r->stuck > 45) {
            r->reverse = 40;
            r->stuck = 0;
            stat_reverses[r - racers]++;
        }
    } else {
        r->stuck = 0;
    }

    /* Last resort: no new point for 5 seconds puts the car back on track. */
    if (r->idle > 300)
        race_rescue(r);
}
