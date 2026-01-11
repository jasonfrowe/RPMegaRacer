#include <rp6502.h>
#include <stdint.h>
#include <stdlib.h>
#include "player.h"
#include "ai.h"
#include "track.h"

#define CAR_PUSH_FORCE 0x080 // 2.0 pixels (Stronger push)
#define IMPACT_SPIN 16       // Large angle jerk (approx 45 degrees)

void resolve_car_collision(Car *c1, Car *c2) {
    int16_t dx = (c2->x >> 6) - (c1->x >> 6);
    int16_t dy = (c2->y >> 6) - (c1->y >> 6);
    
    // Quick Manhattan exit
    if (abs(dx) > 12 || abs(dy) > 12) return;

    uint16_t dist_sq = (dx * dx) + (dy * dy);

    if (dist_sq < 100 && dist_sq > 0) {
        // --- 1. MOMENTUM SWAP ---
        int16_t temp_vx = c1->vel_x;
        int16_t temp_vy = c1->vel_y;
        c1->vel_x = c2->vel_x;
        c1->vel_y = c2->vel_y;
        c2->vel_x = temp_vx;
        c2->vel_y = temp_vy;

        // --- 2. THE SPIN ---
        c1->angle += (rand() % (IMPACT_SPIN * 2)) - IMPACT_SPIN;
        c2->angle += (rand() % (IMPACT_SPIN * 2)) - IMPACT_SPIN;

        // --- 3. WALL-AWARE PHYSICAL SEPARATION ---
        // We only nudge if the destination is NOT a wall.
        // This prevents being flung into or through barriers.
        
        int16_t push_x = (dx > 0) ? -CAR_PUSH_FORCE : CAR_PUSH_FORCE;
        int16_t push_y = (dy > 0) ? -CAR_PUSH_FORCE : CAR_PUSH_FORCE;

        // Check C1: Would the push put C1 in a wall?
        if (get_terrain_at(((c1->x + push_x) >> 6) + 8, ((c1->y + push_y) >> 6) + 8) != TERRAIN_WALL) {
            c1->x += push_x;
            c1->y += push_y;
        }

        // Check C2: Would the push put C2 in a wall?
        if (get_terrain_at(((c2->x - push_x) >> 6) + 8, ((c2->y - push_y) >> 6) + 8) != TERRAIN_WALL) {
            c2->x -= push_x;
            c2->y -= push_y;
        }

        // --- 4. TRIGGER STUN ---
        // Force the cars into the "rebound" state so they can't 
        // immediately drive back into each other.
        // Assuming you add rebound_timer to the AICar struct as well:
        extern uint8_t rebound_timer; 
        rebound_timer = 10; // Stun the player
    }
}

// 1.0 pixel in 10.6 fixed point
#define AI_PUSH_FORCE 0x040 

void resolve_ai_ai_collision(AICar *a1, AICar *a2) {
    // 1. Quick Manhattan Distance check
    // 10.6 world coordinates >> 6 for pixels
    int16_t dx = (int16_t)(a2->car.x >> 6) - (int16_t)(a1->car.x >> 6);
    int16_t dy = (int16_t)(a2->car.y >> 6) - (int16_t)(a1->car.y >> 6);
    
    // Check for 8x8 pixel overlap (half of a car)
    // Using abs() on 16-bit is very fast on 6502
    if (abs(dx) < 8 && abs(dy) < 8) {
        
        // 2. SIMPLE NUDGE (No wall lookups!)
        // If they are on top of each other, force a direction
        if (dx == 0 && dy == 0) dx = 1; 

        // Physically separate them
        if (dx > 0) { a1->car.x -= AI_PUSH_FORCE; a2->car.x += AI_PUSH_FORCE; }
        else       { a1->car.x += AI_PUSH_FORCE; a2->car.x -= AI_PUSH_FORCE; }

        if (dy > 0) { a1->car.y -= AI_PUSH_FORCE; a2->car.y += AI_PUSH_FORCE; }
        else       { a1->car.y += AI_PUSH_FORCE; a2->car.y -= AI_PUSH_FORCE; }

        // 3. MOMENTUM SWAP (Fastest way)
        int16_t tvx = a1->car.vel_x;
        a1->car.vel_x = a2->car.vel_x;
        a2->car.vel_x = tvx;

        int16_t tvy = a1->car.vel_y;
        a1->car.vel_y = a2->car.vel_y;
        a2->car.vel_y = tvy;

        // 4. JITTER
        // Instead of rand(), use the VSync LSB to tweak angles
        // This stops them from locking together in a perfectly straight line
        a1->car.angle += (RIA.vsync & 0x03);
        a2->car.angle -= (RIA.vsync & 0x03);
    }
}