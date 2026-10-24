#ifndef CAR_H
#define CAR_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Units: positions are 1/256 px, velocities 1/256 px per frame, and the
 * heading is a game angle in 8.8 (the high byte is 0..255, 0 = up, 64 = left).
 */

/* Turning, 8.8 game angle units per frame at full lock. Slow cars still
 * turn, so a car pinned against a wall can always turn away. */
#define TURN_SLOW 0x0140 /* standing still */
#define TURN_FAST 0x02C0 /* at SPEED_REF and above */
#define SPEED_REF 512

/* Share of sideways slide removed per frame, out of 256: lower drifts more. */
#define GRIP_ROAD 72
#define GRIP_GRASS 20

/* Engine, 1/16 of 1/256 px per frame per frame. Top speed on the road is
 * about ACCEL / 128 px per frame: 2 px per frame for ACCEL_BASE. */
#define ACCEL_BASE 256
#define DRAG_SHIFT 5
#define GRASS_DRAG_SHIFT 4

/* Of the speed into a wall, the share that bounces back, out of 256. */
#define WALL_BOUNCE 48
/* Sliding along a wall loses 1/2^WALL_FRICTION_SHIFT of the speed into it,
 * over the first WALL_TOUCH, which is free. */
#define WALL_FRICTION_SHIFT 2
#define WALL_TOUCH 48
/* A blocked step that a shift of up to WALL_STEP px clears is a bump in
 * the wall, and the car rides over it. */
#define WALL_STEP 3
/* Walls within WALL_REPEL_R px of a car's centre push it away, by about
 * 4 * WALL_REPEL per frame for a wall on one side. */
#define WALL_REPEL_R 7
#define WALL_REPEL 3
/* A hit within this many angle units of running along the wall turns the
 * car up to WALL_ALIGN_STEP units toward it, each frame it touches. */
#define WALL_ALIGN_RANGE 40
#define WALL_ALIGN_STEP 2

/* Car to car contact: centres closer than this (px) touch. */
#define CAR_RADIUS 5
#define CAR_CONTACT (CAR_RADIUS * 2)

typedef struct
{
    int32_t x, y;     /* centre */
    int16_t vx, vy;
    uint16_t heading;
    int16_t speed;    /* forward speed, set by car_update() */

    /* Controls, set by the driver before car_update(). */
    int8_t steer;     /* -16..16, + turns left */
    bool gas;
    bool brake;       /* brakes, then reverses */
    uint16_t accel;   /* engine power, ACCEL_BASE is normal */

    /* Set by car_update(): non-zero for a few frames after a hit. */
    uint8_t wall_hit;
    uint8_t car_hit;
} Car;

#define PX(v) ((int16_t)((v) >> 8))
#define FROM_PX(p) ((int32_t)(p) << 8)

void car_place(Car *c, int16_t x, int16_t y, uint8_t heading);
void car_update(Car *c);

/* Pushes overlapping cars apart and trades their momentum along the line
 * between them. The share is how much of the push and the knock goes to
 * a, out of 256; b takes the rest. */
void car_contact(Car *a, Car *b, uint8_t share);

bool car_blocked(int16_t px, int16_t py);

/* Moves a car out of a wall it somehow got into. */
void car_unstick(Car *c);

#endif
