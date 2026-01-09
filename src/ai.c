#include "ai.h"
#include "constants.h"
#include "track.h"
#include "player.h"
#include <rp6502.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Integer square root using Newton's method
static uint16_t isqrt(uint32_t n) {
    if (n == 0) return 0;
    uint16_t x = n;
    uint16_t y = (x + 1) >> 1;
    while (y < x) {
        x = y;
        y = (x + n / x) >> 1;
    }
    return x;
}

// Integer atan2 returning angle in 0-255 range (matches our angle system)
// Angle system: 0=Up/North, 64=Left/West, 128=Down/South, 192=Right/East
static uint8_t iatan2(int16_t dy, int16_t dx) {
    // Handle zero cases
    if (dx == 0 && dy == 0) return 0;
    
    // Determine quadrant and calculate angle
    // Our system: X increases right, Y increases down
    // 0 = Up (negative Y), 64 = Left (negative X), 128 = Down (positive Y), 192 = Right (positive X)
    
    uint8_t angle;
    
    // Use absolute values for calculation
    int16_t abs_dx = (dx < 0) ? -dx : dx;
    int16_t abs_dy = (dy < 0) ? -dy : dy;
    
    // Calculate base angle in first octant (0-64)
    if (abs_dx > abs_dy) {
        // More horizontal than vertical
        if (abs_dy == 0) {
            angle = 0;
        } else {
            angle = (abs_dy * 64) / abs_dx;
        }
    } else {
        // More vertical than horizontal
        if (abs_dx == 0) {
            angle = 64;
        } else {
            angle = 64 - (abs_dx * 64) / abs_dy;
        }
    }
    
    // Map to correct quadrant based on signs
    // dx < 0, dy < 0: upper-left quadrant (angles 0-64)
    // dx > 0, dy < 0: upper-right quadrant (angles 192-256/0)
    // dx < 0, dy > 0: lower-left quadrant (angles 64-128)
    // dx > 0, dy > 0: lower-right quadrant (angles 128-192)
    
    if (dy < 0) {
        // Upper half (moving up)
        if (dx < 0) {
            // Upper-left: angle 0-64 (up to left)
            angle = 64 - angle;
        } else {
            // Upper-right: angle 192-256 (right to up)
            angle = 256 - angle;
        }
    } else {
        // Lower half (moving down)
        if (dx < 0) {
            // Lower-left: angle 64-128 (left to down)
            angle = 64 + angle;
        } else {
            // Lower-right: angle 128-192 (down to right)
            angle = 128 + angle;
        }
    }
    
    return angle;
}

AICar ai_cars[NUM_AI_CARS];

// Track waypoints - racing line coordinates
Waypoint waypoints[NUM_WAYPOINTS] = {
    {255, 60},   // 0: Top-Middle (Start/Finish)
    {120, 50},   // 1: Top-Right Entry
    {91, 84},    // 2: Top-Right Apex
    {60, 192},   // 3: Right-Side Mid
    {48, 255},   // 4: Bottom-Right Entry
    {88, 295},   // 5: Bottom-Right Apex
    {247, 315},  // 6: Bottom-Middle
    {375, 328},  // 7: Bottom-Left Entry
    {431, 295},  // 8: Bottom-Left Apex
    {446, 175},  // 9: Left-Side Mid
    {453, 116},  // 10: Top-Left Entry
    {417, 80}    // 11: Top-Left Apex
};

void init_ai(void) {
    // Starting positions on the grid, left of starting line (X=255)
    // Spread across track width (Y=33 to Y=86)
    int16_t start_positions[NUM_AI_CARS] = {40, 52, 64};  // Y positions (front to back)
    
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        ai_cars[i].car.x = 245L << 8;  // X = 245, left of starting line (8.8 fixed point)
        ai_cars[i].car.y = ((int32_t)start_positions[i]) << 8;  // Starting Y
        ai_cars[i].car.vel_x = 0;
        ai_cars[i].car.vel_y = 0;
        ai_cars[i].car.angle = 64;  // Facing Left (West)
        
        ai_cars[i].current_waypoint = 1;  // Start at waypoint 1 (skip start/finish line)
        ai_cars[i].offset_x = (rand() % 20) - 10;  // Random offset ±10
        ai_cars[i].offset_y = (rand() % 20) - 10;
        ai_cars[i].sprite_index = i + 1;  // Sprites 1, 2, 3 (player is 0)
        ai_cars[i].startup_delay = 600;  // 10 seconds at 60fps
    }
}

void update_ai(void) {
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        
        // Handle startup delay
        if (ai->startup_delay > 0) {
            ai->startup_delay--;
            continue;  // Don't drive yet
        }
        
        // Get current waypoint with offset
        int16_t target_x = waypoints[ai->current_waypoint].x + ai->offset_x;
        int16_t target_y = waypoints[ai->current_waypoint].y + ai->offset_y;
        
        // Check if we've reached the current waypoint
        int16_t car_x = ai->car.x >> 8;
        int16_t car_y = ai->car.y >> 8;
        int16_t dx = target_x - car_x;
        int16_t dy = target_y - car_y;
        int32_t dist_sq = (int32_t)dx * dx + (int32_t)dy * dy;
        
        // Switch waypoint if close enough OR if we're moving away from it
        // (prevents getting stuck if we overshoot)
        if (dist_sq < (WAYPOINT_REACH_RADIUS * WAYPOINT_REACH_RADIUS)) {
            // Move to next waypoint
            ai->current_waypoint = (ai->current_waypoint + 1) % NUM_WAYPOINTS;
            // Generate new random offset
            ai->offset_x = (rand() % 20) - 10;
            ai->offset_y = (rand() % 20) - 10;
            
            // Update target
            target_x = waypoints[ai->current_waypoint].x + ai->offset_x;
            target_y = waypoints[ai->current_waypoint].y + ai->offset_y;
            
            // Recalculate for steering
            dx = target_x - car_x;
            dy = target_y - car_y;
        }
        
        // Add look-ahead: steer toward a point 10 pixels past the waypoint
        uint8_t next_wp = (ai->current_waypoint + 1) % NUM_WAYPOINTS;
        int16_t lookahead_dx = waypoints[next_wp].x - waypoints[ai->current_waypoint].x;
        int16_t lookahead_dy = waypoints[next_wp].y - waypoints[ai->current_waypoint].y;
        
        // Normalize and scale by look-ahead distance
        uint32_t la_dist_sq = (uint32_t)(lookahead_dx * lookahead_dx + lookahead_dy * lookahead_dy);
        int16_t la_dist = isqrt(la_dist_sq);
        if (la_dist > 0) {
            target_x += (lookahead_dx * WAYPOINT_LOOKAHEAD) / la_dist;
            target_y += (lookahead_dy * WAYPOINT_LOOKAHEAD) / la_dist;
        }
        
        // Calculate desired angle to target
        dx = target_x - car_x;
        dy = target_y - car_y;
        
        // Use integer atan2 that returns 0-255 angle directly
        uint8_t target_angle = iatan2(dy, dx);  // dy first to match our coordinate system
        
        // Steer toward target angle
        int16_t angle_diff = target_angle - ai->car.angle;
        
        // Handle wrapping (e.g., turning from 250 to 10 should be +16, not -240)
        if (angle_diff > 128) angle_diff -= 256;
        if (angle_diff < -128) angle_diff += 256;
        
        // Apply steering (limit turn rate)
        #define AI_TURN_SPEED 3
        if (angle_diff > AI_TURN_SPEED) {
            ai->car.angle += AI_TURN_SPEED;
        } else if (angle_diff < -AI_TURN_SPEED) {
            ai->car.angle -= AI_TURN_SPEED;
        } else {
            ai->car.angle = target_angle;
        }
        
        // Apply constant forward thrust
        int8_t sin_val = SIN_LUT[ai->car.angle];
        int8_t cos_val = SIN_LUT[(ai->car.angle + 64) & 0xFF];
        
        // AI cars thrust forward constantly (same as player thrust)
        ai->car.vel_x -= (int16_t)sin_val >> THRUST_SCALER;
        ai->car.vel_y -= (int16_t)cos_val >> THRUST_SCALER;
        
        // Apply friction
        int16_t drag_x = (ai->car.vel_x >> FRICTION_SHIFT);
        int16_t drag_y = (ai->car.vel_y >> FRICTION_SHIFT);
        
        if (drag_x == 0 && ai->car.vel_x != 0) {
            drag_x = (ai->car.vel_x > 0) ? 1 : -1;
        }
        if (drag_y == 0 && ai->car.vel_y != 0) {
            drag_y = (ai->car.vel_y > 0) ? 1 : -1;
        }
        
        ai->car.vel_x -= drag_x;
        ai->car.vel_y -= drag_y;
        
        // Apply velocity to position with collision check
        int32_t new_x = ai->car.x + ai->car.vel_x;
        int32_t new_y = ai->car.y + ai->car.vel_y;
        
        // Simple collision check (reuse from player)
        extern uint8_t check_collision_at_pos(int32_t x, int32_t y, uint8_t angle);
        if (check_collision_at_pos(new_x, new_y, ai->car.angle) == TERRAIN_WALL) {
            // Bounce
            ai->car.vel_x = -(ai->car.vel_x >> 2);
            ai->car.vel_y = -(ai->car.vel_y >> 2);
        } else {
            ai->car.x = new_x;
            ai->car.y = new_y;
        }
        
        // Clamp to world bounds
        if (ai->car.x < 0) ai->car.x = 0;
        if (ai->car.x > 131072L) ai->car.x = 131072L;
        if (ai->car.y < 0) ai->car.y = 0;
        if (ai->car.y > 98304L) ai->car.y = 98304L;
    }
}

void draw_ai_cars(void) {
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        unsigned config_addr = REDRACER_CONFIG + sizeof(vga_mode4_asprite_t) * ai->sprite_index;
        
        // Get camera position from player
        extern Car car;  // Player car
        int16_t camera_x = (car.x >> 8) - 160;
        int16_t camera_y = (car.y >> 8) - 120;
        
        if (camera_x < 0) camera_x = 0;
        if (camera_x > 192) camera_x = 192;
        if (camera_y < 0) camera_y = 0;
        if (camera_y > 144) camera_y = 144;
        
        // Calculate screen position
        int16_t screen_x = (ai->car.x >> 8) - camera_x;
        int16_t screen_y = (ai->car.y >> 8) - camera_y;
        
        // Set rotation matrix
        int16_t s = (int16_t)SIN_LUT[ai->car.angle] << 1;
        int16_t c = (int16_t)SIN_LUT[(ai->car.angle + 64) & 0xFF] << 1;
        
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[0], c);  // SX
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[1], -s); // SHY
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[3], s);  // SHX
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[4], c);  // SY
        
        // Pivot translation
        int16_t tx = 8 * (256 - c + s);
        int16_t ty = 8 * (256 - c - s);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[2], tx);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[5], ty);
        
        // Update position
        xram0_struct_set(config_addr, vga_mode4_asprite_t, x_pos_px, screen_x);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, y_pos_px, screen_y);
    }
}
