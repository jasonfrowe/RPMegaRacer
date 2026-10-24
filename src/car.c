#include <stdlib.h>
#include "car.h"
#include "track.h"
#include "trig.h"

/* v * k / 128 for a sine-table k, in 16 bits. */
static int16_t mulsc(int16_t v, int8_t k)
{
    if (v > -256 && v < 256)
        return (int16_t)((v * k) >> 7);
    return (int16_t)(((v >> 3) * k) >> 4); /* |v| < 2048 */
}

/* v minus v / 2^sh, rounded toward zero so it never creeps. */
static int16_t decay(int16_t v, uint8_t sh)
{
    return v >= 0 ? v - (v >> sh) : v + ((-v) >> sh);
}

/* The car's outline against the walls: 8 points on a circle of radius 5 px.
 * Called a lot, so it works in wall cells (2 px) and bytes. */
static inline bool cell(const uint8_t *row, uint8_t cx)
{
    return row[cx >> 3] & bit_mask[cx & 7];
}

bool car_blocked(int16_t px, int16_t py)
{
    /* The edge of the world is wall; cars never get near it. */
    if ((uint16_t)(px - 6) >= WORLD_W - 12 || (uint16_t)(py - 6) >= WORLD_H - 12)
        return true;
    uint8_t xm5 = (uint8_t)((uint16_t)(px - 5) >> 1);
    uint8_t xm4 = (uint8_t)((uint16_t)(px - 4) >> 1);
    uint8_t x0 = (uint8_t)((uint16_t)px >> 1);
    uint8_t xp4 = (uint8_t)((uint16_t)(px + 4) >> 1);
    uint8_t xp5 = (uint8_t)((uint16_t)(px + 5) >> 1);
    const uint8_t *r = walls[(uint16_t)(py - 5) >> 1];
    if (cell(r, x0))
        return true;
    r = walls[(uint16_t)(py - 4) >> 1];
    if (cell(r, xm4) || cell(r, xp4))
        return true;
    r = walls[(uint16_t)py >> 1];
    if (cell(r, xm5) || cell(r, xp5))
        return true;
    r = walls[(uint16_t)(py + 4) >> 1];
    if (cell(r, xm4) || cell(r, xp4))
        return true;
    r = walls[(uint16_t)(py + 5) >> 1];
    return cell(r, x0);
}

static const int8_t dir8_x[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int8_t dir8_y[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

void car_unstick(Car *c)
{
    int16_t px = PX(c->x);
    int16_t py = PX(c->y);
    for (int8_t r = 1; r <= 16; r++) {
        for (uint8_t d = 0; d < 8; d++) {
            int16_t nx = px + dir8_x[d] * r;
            int16_t ny = py + dir8_y[d] * r;
            if (!car_blocked(nx, ny)) {
                c->x = FROM_PX(nx) + 128;
                c->y = FROM_PX(ny) + 128;
                return;
            }
        }
    }
}

void car_place(Car *c, int16_t x, int16_t y, uint8_t heading)
{
    c->x = FROM_PX(x) + 128;
    c->y = FROM_PX(y) + 128;
    c->vx = c->vy = 0;
    c->speed = 0;
    c->heading = (uint16_t)heading << 8;
    c->steer = 0;
    c->gas = c->brake = false;
    c->wall_hit = c->car_hit = 0;
    if (car_blocked(x, y))
        car_unstick(c);
}

#define V_MAX 1000

void car_update(Car *c)
{
    bool on_grass = grass_at(PX(c->x), PX(c->y));

    /* Steer: quicker at speed, but never stuck at zero. */
    int16_t sp = c->speed < 0 ? -c->speed : c->speed;
    if (sp > SPEED_REF)
        sp = SPEED_REF;
    int16_t rate = TURN_SLOW + (int16_t)((((sp >> 1) * ((TURN_FAST - TURN_SLOW) >> 3))) >> 5);
    c->heading += (int16_t)((rate * c->steer) >> 4);

    uint8_t a = (uint8_t)(c->heading >> 8);
    int8_t s = SIN(a);
    int8_t co = COS(a);
    int16_t vx = c->vx;
    int16_t vy = c->vy;

    /* Grip: bleed off the sideways part of the velocity. Right of the
     * heading is (cos, -sin). */
    int16_t lat = mulsc(vx, co) - mulsc(vy, s);
    int16_t dl = (int16_t)(((lat >> 2) * (on_grass ? GRIP_GRASS : GRIP_ROAD)) >> 6);
    vx -= mulsc(dl, co);
    vy += mulsc(dl, s);

    /* Engine and brakes. Forward is (-sin, -cos). */
    uint16_t power = 0;
    int8_t dir = 0;
    if (c->gas) {
        power = c->accel;
        dir = 1;
    } else if (c->brake) {
        /* Brakes bite harder than the engine; once stopped, reverse slowly. */
        power = c->speed > 64 ? c->accel + (c->accel >> 1) : c->accel >> 1;
        dir = -1;
    }
    if (power) {
        int16_t p = (int16_t)(power >> 1);
        int16_t tx = (int16_t)((p * s) >> 10);
        int16_t ty = (int16_t)((p * co) >> 10);
        if (dir > 0) {
            vx -= tx;
            vy -= ty;
        } else {
            vx += tx;
            vy += ty;
        }
    }

    /* Drag sets the top speed. */
    vx = decay(vx, DRAG_SHIFT);
    vy = decay(vy, DRAG_SHIFT);
    if (on_grass) {
        vx = decay(vx, GRASS_DRAG_SHIFT);
        vy = decay(vy, GRASS_DRAG_SHIFT);
    }
    if (!power && vx > -8 && vx < 8 && vy > -8 && vy < 8)
        vx = vy = 0;
    if (vx > V_MAX) vx = V_MAX;
    if (vx < -V_MAX) vx = -V_MAX;
    if (vy > V_MAX) vy = V_MAX;
    if (vy < -V_MAX) vy = -V_MAX;

    /* Move one axis at a time, at most 2 px (one wall cell) per step, so a
     * car slides along a wall instead of sticking to it. */
    int16_t ax = vx < 0 ? -vx : vx;
    int16_t ay = vy < 0 ? -vy : vy;
    uint8_t steps = ((ax > ay ? ax : ay) > 512) ? 2 : 1;
    int16_t stx = steps == 2 ? vx / 2 : vx;
    int16_t sty = steps == 2 ? vy / 2 : vy;
    bool hit_x = false, hit_y = false;
    for (uint8_t i = 0; i < steps; i++) {
        if (!hit_x && stx) {
            int32_t nx = c->x + stx;
            if (car_blocked(PX(nx), PX(c->y)))
                hit_x = true;
            else
                c->x = nx;
        }
        if (!hit_y && sty) {
            int32_t ny = c->y + sty;
            if (car_blocked(PX(c->x), PX(ny)))
                hit_y = true;
            else
                c->y = ny;
        }
    }
    if (hit_x) {
        vx = -(int16_t)(((vx >> 2) * WALL_BOUNCE) >> 6);
        vy = decay(vy, WALL_SCRAPE);
        c->wall_hit = 8;
    }
    if (hit_y) {
        vy = -(int16_t)(((vy >> 2) * WALL_BOUNCE) >> 6);
        vx = decay(vx, WALL_SCRAPE);
        c->wall_hit = 8;
    }

    c->vx = vx;
    c->vy = vy;
    c->speed = -(mulsc(vx, s) + mulsc(vy, co));

    if (c->wall_hit)
        c->wall_hit--;
    if (c->car_hit)
        c->car_hit--;
}

static void try_shift(Car *c, int16_t dx, int16_t dy)
{
    int32_t nx = c->x + dx;
    int32_t ny = c->y + dy;
    if (!car_blocked(PX(nx), PX(ny))) {
        c->x = nx;
        c->y = ny;
    } else if (!car_blocked(PX(nx), PX(c->y))) {
        c->x = nx;
    } else if (!car_blocked(PX(c->x), PX(ny))) {
        c->y = ny;
    }
}

/* Of a car-to-car hit, the share of the closing speed that bounces back,
 * out of 256. Low, so cars nudge rather than ping off each other. */
#define CAR_BOUNCE 40

/* a * b / 256 for |a| < 2048 and |b| <= 256, in 16 bits. */
static int16_t mul256(int16_t a, int16_t b)
{
    return (int16_t)(((a >> 3) * (b >> 1)) >> 4);
}

void car_contact(Car *a, Car *b, uint8_t share)
{
    int32_t ddx = b->x - a->x;
    int32_t ddy = b->y - a->y;
    const int32_t reach = (int32_t)CAR_CONTACT << 8;
    if (ddx >= reach || ddx <= -reach || ddy >= reach || ddy <= -reach)
        return;

    /* 1/8 px from here on. */
    int16_t dx = (int16_t)(ddx >> 5);
    int16_t dy = (int16_t)(ddy >> 5);
    const int16_t contact = CAR_CONTACT * 8;
    uint16_t d2 = (uint16_t)(dx * dx) + (uint16_t)(dy * dy);
    if (d2 >= (uint16_t)(contact * contact))
        return;

    /* The unit normal from a to b, by angle so there's no divide. */
    uint8_t ang = (dx == 0 && dy == 0) ? 0 : atan2_8(dy, dx);
    int16_t nx = (int16_t)COS(ang) * 2; /* 256 = 1 */
    int16_t ny = (int16_t)SIN(ang) * 2;
    int16_t dist = (int16_t)((dx * nx + dy * ny) >> 8);
    if (dist < 0)
        dist = 0;

    /* Separate: the overlap in 1/256 px, split by share. */
    int16_t overlap = (contact - dist) * 32 + 16;
    if (overlap > 2000)
        overlap = 2000;
    int16_t pa = (int16_t)(((uint16_t)(overlap >> 3) * share) >> 5);
    int16_t pb = overlap - pa;
    try_shift(a, -mul256(pa, nx), -mul256(pa, ny));
    try_shift(b, mul256(pb, nx), mul256(pb, ny));

    /* Closing speed along the normal; only push if they're closing. */
    int16_t closing = mul256(b->vx - a->vx, nx) + mul256(b->vy - a->vy, ny);
    if (closing < 0) {
        int16_t j = -closing;
        if (j > 1600)
            j = 1600;
        j += (int16_t)(((uint16_t)(j >> 2) * CAR_BOUNCE) >> 6);
        int16_t ja = (int16_t)(((uint16_t)(j >> 3) * share) >> 5);
        int16_t jb = j - ja;
        a->vx -= mul256(ja, nx);
        a->vy -= mul256(ja, ny);
        b->vx += mul256(jb, nx);
        b->vy += mul256(jb, ny);
        if (j > 96) {
            a->car_hit = 8;
            b->car_hit = 8;
        }
    }
}
