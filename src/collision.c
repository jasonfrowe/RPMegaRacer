#include <stdint.h>
#include <stdlib.h>
#include "player.h"
#include "ai.h"
#include "track.h"

#define CAR_PUSH_FORCE 0x080 // 2.0 pixels (Stronger push)
#define IMPACT_SPIN 16       // Large angle jerk (approx 45 degrees)

void resolve_car_collision(Car *c1, Car *c2) {
    int16_t dx = (c2->x >> 8) - (c1->x >> 8);
    int16_t dy = (c2->y >> 8) - (c1->y >> 8);
    
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
        if (get_terrain_at(((c1->x + push_x) >> 8) + 8, ((c1->y + push_y) >> 8) + 8) != TERRAIN_WALL) {
            c1->x += push_x;
            c1->y += push_y;
        }

        // Check C2: Would the push put C2 in a wall?
        if (get_terrain_at(((c2->x - push_x) >> 8) + 8, ((c2->y - push_y) >> 8) + 8) != TERRAIN_WALL) {
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