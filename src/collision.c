#include <rp6502.h>
#include <stdint.h>
#include <stdlib.h>
#include "player.h"
#include "ai.h"
#include "track.h"

// Physics Tuning
#define PLAYER_PUSH_FORCE 0x0C0 // 1.0 pixel (Player resists push)
#define AI_PUSH_FORCE_HVY 0x0C0 // 3.0 pixels (AI gets shoved)
#define PLAYER_SPIN       8     // Minor twitch
#define AI_SPIN           8    // Huge spinout

// Rebound Timers (frames of "stun")
#define PLAYER_STUN       5
#define AI_STUN           10

void resolve_player_ai_collision(Car *p, AICar *ai) {
    Car *c2 = &ai->car; // Short alias for the AI's physics car

    int16_t dx = (c2->x >> 6) - (p->x >> 6);
    int16_t dy = (c2->y >> 6) - (p->y >> 6);
    
    // Quick Manhattan exit
    if (abs(dx) > 14 || abs(dy) > 14) return; // Slightly larger detection radius for high speed

    uint16_t dist_sq = (dx * dx) + (dy * dy);

    if (dist_sq < 140 && dist_sq > 0) { // 140 = ~11.8px distance
        
        // --- 1. MOMENTUM SWAP (Weighted) ---
        // Player keeps 50% of their momentum + 50% of AI's
        // AI takes 100% of Player's momentum
        int16_t p_vx = p->vel_x;
        int16_t p_vy = p->vel_y;
        
        p->vel_x = (p->vel_x >> 1) + (c2->vel_x >> 1);
        p->vel_y = (p->vel_y >> 1) + (c2->vel_y >> 1);
        
        c2->vel_x = p_vx; // AI gets slammed with full player force
        c2->vel_y = p_vy;

        // --- 2. THE SPIN ---
        p->angle += (rand() % (PLAYER_SPIN * 2)) - PLAYER_SPIN;
        c2->angle += (rand() % (AI_SPIN * 2)) - AI_SPIN;

        // --- 3. PHYSICAL SEPARATION (Asymmetric) ---
        int16_t push_x = (dx > 0) ? -1 : 1;
        int16_t push_y = (dy > 0) ? -1 : 1;

        // Player gets nudged lightly
        int16_t p_push_x = push_x * PLAYER_PUSH_FORCE;
        int16_t p_push_y = push_y * PLAYER_PUSH_FORCE;
        
        if (get_terrain_at(((p->x + p_push_x) >> 6) + 8, ((p->y + p_push_y) >> 6) + 8) != TERRAIN_WALL) {
            p->x += p_push_x;
            p->y += p_push_y;
        }

        // AI gets shoved hard
        int16_t ai_push_x = -(push_x * AI_PUSH_FORCE_HVY); // Opposite direction
        int16_t ai_push_y = -(push_y * AI_PUSH_FORCE_HVY);

        if (get_terrain_at(((c2->x + ai_push_x) >> 6) + 8, ((c2->y + ai_push_y) >> 6) + 8) != TERRAIN_WALL) {
            c2->x += ai_push_x;
            c2->y += ai_push_y;
        }

        // --- 4. TRIGGER STUN ---
        extern uint8_t rebound_timer; // From player.c
        rebound_timer = PLAYER_STUN;
        ai->rebound_timer = AI_STUN;
        
        // Audio Feedack (Optional)
        // play_crash_sound();
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