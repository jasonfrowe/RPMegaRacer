#ifndef TRACK_H
#define TRACK_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_TRACKS 3

/* World size in pixels. */
#define WORLD_W 512
#define WORLD_H 384

/* Walls at 2x2 pixel cells (see tools/make_track.py). */
#define WALL_COLS (WORLD_W / 2 / 8)
#define WALL_ROWS (WORLD_H / 2)
extern uint8_t walls[WALL_ROWS][WALL_COLS];

/* Grass, one bit per 8x8 tile. */
extern uint8_t grass[48][8];

/* The racing line. */
#define PATH_MAX 250
extern uint8_t path_n;
extern int16_t path_x[PATH_MAX];
extern int16_t path_y[PATH_MAX];
extern uint8_t path_heading[PATH_MAX];
extern uint8_t path_room_l[PATH_MAX];
extern uint8_t path_room_r[PATH_MAX];
extern uint8_t path_speed[PATH_MAX]; /* 1/64 px per frame */

extern uint8_t current_track; /* 1..NUM_TRACKS */

void track_load(uint8_t id);

static inline uint8_t path_next(uint8_t i) { return (uint8_t)(i + 1 == path_n ? 0 : i + 1); }
static inline uint8_t path_prev(uint8_t i) { return (uint8_t)(i == 0 ? path_n - 1 : i - 1); }
static inline uint8_t path_add(uint8_t i, uint8_t k)
{
    uint16_t j = (uint16_t)i + k;
    return (uint8_t)(j >= path_n ? j - path_n : j);
}

extern const uint8_t bit_mask[8];

/* Is pixel (x, y) in a wall? Outside the world counts as wall. */
static inline bool wall_at(int16_t x, int16_t y)
{
    if ((uint16_t)x >= WORLD_W || (uint16_t)y >= WORLD_H)
        return true;
    uint8_t cx = (uint8_t)((uint16_t)x >> 1);
    return walls[(uint16_t)y >> 1][cx >> 3] & bit_mask[cx & 7];
}

static inline bool grass_at(int16_t x, int16_t y)
{
    if ((uint16_t)x >= WORLD_W || (uint16_t)y >= WORLD_H)
        return false;
    uint8_t tx = (uint8_t)((uint16_t)x >> 3);
    return grass[(uint16_t)y >> 3][tx >> 3] & bit_mask[tx & 7];
}

/* Nearest racing line point to (x, y), searching the whole loop. */
uint8_t path_nearest(int16_t x, int16_t y);

#endif
