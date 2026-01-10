#include <stdint.h>
#include <stdlib.h>
#include "player.h"
#include "ai.h"

// Threshold: Cars are 16x16. 
// A distance of 10 pixels squared (100) is a good "sweet spot" for bumping.
#define CAR_COL_DIST_SQ 100 
#define CAR_PUSH_FORCE 0x160 // 1.0 pixel nudge

void resolve_car_collision(Car *c1, Car *c2) {
    // 1. Get pixel coordinates
    int16_t x1 = c1->x >> 8;
    int16_t y1 = c1->y >> 8;
    int16_t x2 = c2->x >> 8;
    int16_t y2 = c2->y >> 8;

    // 2. Quick Manhattan distance check (Optimization for 6502)
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    if (abs(dx) > 12 || abs(dy) > 12) return;

    // 3. Precise distance squared check
    uint16_t dist_sq = (dx * dx) + (dy * dy);

    if (dist_sq < CAR_COL_DIST_SQ && dist_sq > 0) {
        // --- WE HAVE A HIT ---

        // Randomly nudge their angles slightly on impact
        c1->angle += (rand() % 5) - 2;
        c2->angle += (rand() % 5) - 2;

        // A. MOMENTUM SWAP (Arcade Style)
        // This makes them "bounce" off each other by exchanging velocities
        int16_t temp_vx = c1->vel_x;
        int16_t temp_vy = c1->vel_y;

        // Dampen the swap slightly so they don't fly off too wildly
        c1->vel_x = c2->vel_x >> 1;
        c1->vel_y = c2->vel_y >> 1;
        c2->vel_x = temp_vx >> 1;
        c2->vel_y = temp_vy >> 1;

        // B. POSITIONAL NUDGE (Anti-fusing)
        // Physically push them apart so they don't get stuck inside each other
        if (dx > 0) {
            c1->x -= CAR_PUSH_FORCE;
            c2->x += CAR_PUSH_FORCE;
        } else {
            c1->x += CAR_PUSH_FORCE;
            c2->x -= CAR_PUSH_FORCE;
        }

        if (dy > 0) {
            c1->y -= CAR_PUSH_FORCE;
            c2->y += CAR_PUSH_FORCE;
        } else {
            c1->y += CAR_PUSH_FORCE;
            c2->y -= CAR_PUSH_FORCE;
        }

        // C. Trigger SFX (PSG Crunch/Clink)
        // play_psg_bump(); 
    }
}