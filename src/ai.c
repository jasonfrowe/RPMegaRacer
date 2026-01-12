#include "ai.h"
#include "constants.h"
#include "track.h"
#include "player.h"
#include <rp6502.h>
#include <stdlib.h>
#include "racelogic.h"
#include "hud.h"
#include "input.h"
#include <stdio.h>

#define AI_TURN_SPEED 3
#define AI_MAX_THRUST_SHIFT 3      
#define AI_REDUCED_THRUST_SHIFT 4  

#define BOUNCE_IMPULSE 0x080  
#define PUSH_OUT_10_6  0x018  // ~0.4 pixels in 10.6 (0.4 * 64)
#define REBOUND_STUN   4       

// Shared optimization: 4-corner check using 16-bit pixels
static uint8_t is_colliding_ai(int16_t px, int16_t py) {
    int16_t cx = px + 8; // Center of 16x16
    int16_t cy = py + 8;
    #define H 5 // 10x10 hitbox

    // Corners
    if (get_terrain_at(cx - H, cy - H) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx + H, cy - H) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx - H, cy + H) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx + H, cy + H) == TERRAIN_WALL) return 1;
    // Midpoints
    if (get_terrain_at(cx,     cy - H) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx,     cy + H) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx - H, cy    ) == TERRAIN_WALL) return 1;
    if (get_terrain_at(cx + H, cy    ) == TERRAIN_WALL) return 1;

    return 0;
}

uint8_t atan2_8(int16_t dy, int16_t dx) {
    if (dx == 0 && dy == 0) return 0;
    int16_t abs_dx = (dx < 0) ? -dx : dx;
    int16_t abs_dy = (dy < 0) ? -dy : dy;
    uint8_t angle;
    if (abs_dx > abs_dy) {
        angle = (abs_dy == 0) ? 0 : (abs_dy * 64) / abs_dx;
    } else {
        angle = (abs_dx == 0) ? 64 : 64 - (abs_dx * 64) / abs_dy;
    }
    if (dx >= 0 && dy >= 0) return angle;
    if (dx < 0 && dy >= 0) return 128 - angle;
    if (dx < 0 && dy < 0) return 128 + angle;
    return 256 - angle;
}

AICar ai_cars[NUM_AI_CARS];

Waypoint waypoints[NUM_WAYPOINTS];

void init_ai(void) {
    int16_t start_positions[NUM_AI_CARS] = {40, 50, 60};  
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        ai_cars[i].car.x = 245 << 6;  // 10.6 init
        ai_cars[i].car.y = start_positions[i] << 6;
        ai_cars[i].car.vel_x = 0;
        ai_cars[i].car.vel_y = 0;
        ai_cars[i].car.angle = 64;  
        ai_cars[i].current_waypoint = 1;  
        ai_cars[i].sprite_index = i + 1;  
        ai_cars[i].last_recorded_x = 245;  
        ai_cars[i].last_recorded_y = start_positions[i];
        ai_cars[i].base_speed_shift = AI_SPEED_NORMAL;
        ai_cars[i].last_thrust_shift = AI_SPEED_NORMAL;
    }
    car.total_progress = 0;
    car.current_waypoint = 1;
}

static uint8_t ai_brain_turn = 0; // Rotates 0, 1, 2

void update_ai(void) {
    // Manage which AI car gets to "think" this frame
    if (++ai_brain_turn >= NUM_AI_CARS) ai_brain_turn = 0;

    // Check for the Start Trigger
    if (!countdown_active) {
        hud_print(13, 5, " PRESS FIRE TO START ", HUD_COL_WHITE, HUD_COL_BG);
        if (is_action_pressed(0, ACTION_FIRE)) {
            countdown_active = true;
            // Clear the "Press Fire" message
            hud_print(13, 5, "                     ", 0, 0);
        }
        return; // Don't do anything else until they press Fire
    }

    // Global Timer Management (Only runs once per frame)
    if (state_timer > 0) {
        state_timer--;
        update_countdown_display(state_timer);
    }

    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        // AI remains stationary unless racing
        // If the countdown is still running, AI stays still
        if (state_timer > 300) {
            // Optional: reset velocity here to ensure they don't creep
            ai->car.vel_x = 0;
            ai->car.vel_y = 0;
            continue;
        }

        // --- 1. REBOUND TIMER (Every Frame) ---
        // We keep this so they still "stun" when hitting things
        if (ai->rebound_timer > 0) ai->rebound_timer--;

        // --- 2. BRAIN (1 of 3 cars per frame) ---
        if (i == ai_brain_turn) {
            int16_t car_px_x = ai->car.x >> 6;
            int16_t car_px_y = ai->car.y >> 6;

            // --- STUCK DETECTION & RESCUE ---
            ai->stuck_timer++;
            if (ai->stuck_timer >= 30) {
                ai->stuck_timer = 0;
                
                // If the car moved less than 3 pixels in the last 30 frames
                if (abs(car_px_x - ai->last_recorded_x) < 3 && abs(car_px_y - ai->last_recorded_y) < 3) {
                    
                    // --- RESCUE TELEPORT ---
                    // Find the waypoint they just came from
                    uint8_t prev_wp = (ai->current_waypoint == 0) ? (g_num_active_waypoints - 1) : (ai->current_waypoint - 1);
                    
                    // Snap to the center of the previous waypoint
                    ai->car.x = (uint16_t)waypoints[prev_wp].x << 6;
                    ai->car.y = (uint16_t)waypoints[prev_wp].y << 6;
                    
                    // Reset physics so they don't carry "wall-stuck" velocity to the new spot
                    ai->car.vel_x = 0;
                    ai->car.vel_y = 0;
                    ai->rebound_timer = 0;
                }
                
                // Update tracker for the next check
                ai->last_recorded_x = car_px_x;
                ai->last_recorded_y = car_px_y;
            }

            // --- WAYPOINT UPDATING ---
            int16_t dx = waypoints[ai->current_waypoint].x + ai->offset_x - car_px_x;
            int16_t dy = waypoints[ai->current_waypoint].y + ai->offset_y - car_px_y;
            
            // Manhattan distance for VSync speed
            if ((abs(dx) + abs(dy)) < 50) {
                ai->car.progress_steps++; // This never resets!
                ai->current_waypoint++;
                if (ai->current_waypoint >= g_num_active_waypoints) {
                    ai->current_waypoint = 0;
                }
            }
            
            ai->target_angle = (192 - atan2_8(dy, dx)) & 0xFF;
            // uint8_t angle_diff = abs((int8_t)(ai->target_angle - ai->car.angle));
            // ai->last_thrust_shift = (angle_diff > 32) ? AI_REDUCED_THRUST_SHIFT : AI_MAX_THRUST_SHIFT;
            // DYNAMIC SPEED CONTROL
            // Start with the speed set by rubberbanding
            // uint8_t angle_diff = abs((int8_t)(ai->target_angle - ai->car.angle));

            // if (angle_diff > 32) {
            //     // Sharp turn: Slow down by ONE tier
            //     ai->last_thrust_shift = ai->base_speed_shift + 1;
            // } else {
            //     ai->last_thrust_shift = ai->base_speed_shift;
            // }

            // Inside update_ai -> brain turn (if i == ai_brain_turn)
            uint8_t angle_diff = abs((int8_t)(ai->target_angle - ai->car.angle));

            if (angle_diff > 32) {
                // If turning, go one tier slower than the rubberband says
                ai->last_thrust_shift = ai->base_speed_shift + 1;
            } else {
                ai->last_thrust_shift = ai->base_speed_shift;
            }

            // Safety cap: Never go slower than Shift 5
            if (ai->last_thrust_shift > AI_SPEED_SLOW) {
                ai->last_thrust_shift = AI_SPEED_SLOW;
            }

        }

        // --- 3. PHYSICS (Every Frame) ---
        // Turn toward target
        if (state_timer < 270) { // Make a clean start after countdown
            uint8_t diff = ai->target_angle - ai->car.angle;
            if (diff != 0) {
                if (diff < 128) ai->car.angle += AI_TURN_SPEED;
                else ai->car.angle -= AI_TURN_SPEED;
            }
        }

        // Thrust
        if (ai->rebound_timer == 0) {
            int8_t s = SIN_LUT[ai->car.angle];
            int8_t c = SIN_LUT[(ai->car.angle + 64) & 0xFF];
            ai->car.vel_x -= (int16_t)s >> ai->last_thrust_shift;
            ai->car.vel_y -= (int16_t)c >> ai->last_thrust_shift;
        }

        // Friction
        int16_t dvx = (ai->car.vel_x >> FRICTION_SHIFT);
        int16_t dvy = (ai->car.vel_y >> FRICTION_SHIFT);
        if (dvx == 0 && ai->car.vel_x != 0) dvx = (ai->car.vel_x > 0) ? 1 : -1;
        if (dvy == 0 && ai->car.vel_y != 0) dvy = (ai->car.vel_y > 0) ? 1 : -1;
        ai->car.vel_x -= dvx; 
        ai->car.vel_y -= dvy;

        // --- 4. MOVEMENT & WALL COLLISION ---
        int16_t px = (int16_t)(ai->car.x >> 6);
        int16_t py = (int16_t)(ai->car.y >> 6);

        if (ai->car.vel_x != 0) {
            int16_t dx_10_6 = ai->car.vel_x >> 2;
            int16_t nx = (int16_t)((ai->car.x + dx_10_6) >> 6);
            if (is_colliding_ai(nx, py)) {
                ai->car.vel_x = (ai->car.vel_x > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
                ai->car.x += (ai->car.vel_x > 0 ? PUSH_OUT_10_6 : -PUSH_OUT_10_6);
                ai->rebound_timer = REBOUND_STUN;
            } else {
                ai->car.x += dx_10_6;
                px = nx;
            }
        }
        if (ai->car.vel_y != 0) {
            int16_t dy_10_6 = ai->car.vel_y >> 2;
            int16_t ny = (int16_t)((ai->car.y + dy_10_6) >> 6);
            if (is_colliding_ai(px, ny)) {
                ai->car.vel_y = (ai->car.vel_y > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
                ai->car.y += (ai->car.vel_y > 0 ? PUSH_OUT_10_6 : -PUSH_OUT_10_6);
                ai->rebound_timer = REBOUND_STUN;
            } else {
                ai->car.y += dy_10_6;
            }
        }

        // Clamp world bounds (10.6: 512*64 = 32768, 384*64 = 24576)
        if (ai->car.x < 0x200) ai->car.x = 0x200;
        if (ai->car.x > 32768 - 0x200) ai->car.x = 32768 - 0x200;
        if (ai->car.y < 0x200) ai->car.y = 0x200;
        if (ai->car.y > 24576 - 0x200) ai->car.y = 24576 - 0x200;
    }
}

// Optimized Sequential Writes for AI
void draw_ai_cars(int16_t scroll_x, int16_t scroll_y) {
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        uint8_t ang = ai->car.angle;
        
        int16_t s = (int16_t)SIN_LUT[ang] << 1;
        int16_t c = (int16_t)SIN_LUT[(ang + 64) & 0xFF] << 1;
        int16_t tx = TX_LUT[ang];
        int16_t ty = TY_LUT[ang];
        
        // Pixel coordinates (10.6 >> 6)
        int16_t sx = (int16_t)(ai->car.x >> 6) + scroll_x;
        int16_t sy = (int16_t)(ai->car.y >> 6) + scroll_y;

        RIA.addr0 = REDRACER_CONFIG + (sizeof(vga_mode4_asprite_t) * ai->sprite_index);
        RIA.step0 = 1;

        RIA.rw0 = c & 0xFF;    RIA.rw0 = c >> 8;     // SX
        RIA.rw0 = (-s) & 0xFF; RIA.rw0 = (-s) >> 8;  // SHY
        RIA.rw0 = tx & 0xFF;   RIA.rw0 = tx >> 8;    // TX
        RIA.rw0 = s & 0xFF;    RIA.rw0 = s >> 8;     // SHX
        RIA.rw0 = c & 0xFF;    RIA.rw0 = c >> 8;     // SY
        RIA.rw0 = ty & 0xFF;   RIA.rw0 = ty >> 8;    // TY
        RIA.rw0 = sx & 0xFF;   RIA.rw0 = sx >> 8;    
        RIA.rw0 = sy & 0xFF;   RIA.rw0 = sy >> 8;    
    }
}


void update_ai_rubberbanding(AICar *ai) {
    static int16_t last_diff[3] = {0,0,0};
    
    // Calculate difference using the monotonic counters
    int16_t diff = (int16_t)car.progress_steps - (int16_t)ai->car.progress_steps;

    // diff > 0: Player is ahead (AI Speeds up)
    // diff < 0: AI is ahead (AI Slows down)

    if (diff > 1) {
        ai->base_speed_shift = AI_SPEED_FAST; // 2
    } else if (diff < -1) {
        ai->base_speed_shift = AI_SPEED_SLOW;      // 4
    } else {
        ai->base_speed_shift = AI_SPEED_NORMAL;    // 3
    }

    // Diagnostic logging (No more lap glitches!)
    uint8_t id = ai->sprite_index - 1;
    if (diff != last_diff[id]) {
        printf("Car %d: Diff %d | Shift %d | P_Steps: %d | AI_Steps: %d\n", 
               id, diff, ai->base_speed_shift, car.progress_steps, ai->car.progress_steps);
        last_diff[id] = diff;
    }
}

