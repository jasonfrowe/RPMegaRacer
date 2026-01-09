#include "ai.h"
#include "constants.h"
#include "track.h"
#include "player.h"
#include <rp6502.h>
#include <stdlib.h>

#define AI_TURN_SPEED 3
#define AI_MAX_ACCEL 1      // Full thrust scaler
#define AI_REDUCED_ACCEL 2  // Reduced thrust scaler for turns

// Calculate the shortest way to turn from current_angle to target_angle
static void ai_steer_toward(AICar *ai, uint8_t target_angle) {
    // 8-bit math handles wrap-around automatically
    uint8_t diff = target_angle - ai->car.angle;

    if (diff == 0) return;

    // If diff is 1-127, target is to the "right" (CW)
    // If diff is 128-255, target is to the "left" (CCW)
    if (diff < 128) {
        ai->car.angle += AI_TURN_SPEED;
    } else {
        ai->car.angle -= AI_TURN_SPEED;
    }
}

// Apply thrust with variable acceleration
static void ai_apply_thrust(AICar *ai, uint8_t thrust_scaler) {
    // Use SIN_LUT from player module
    extern const int8_t SIN_LUT[256];
    
    int8_t sin_val = SIN_LUT[ai->car.angle];
    int8_t cos_val = SIN_LUT[(ai->car.angle + 64) & 0xFF];
    
    // Apply thrust (same as player but with variable strength)
    ai->car.vel_x -= (int16_t)sin_val >> thrust_scaler;
    ai->car.vel_y -= (int16_t)cos_val >> thrust_scaler;
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
    extern const int8_t SIN_LUT[256];
    
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        
        // Handle startup delay
        if (ai->startup_delay > 0) {
            ai->startup_delay--;
            continue;  // Don't drive yet
        }
        
        // Get current car position
        int16_t car_x = ai->car.x >> 8;
        int16_t car_y = ai->car.y >> 8;
        
        // Get current waypoint target (with offset)
        int16_t target_x = waypoints[ai->current_waypoint].x + ai->offset_x;
        int16_t target_y = waypoints[ai->current_waypoint].y + ai->offset_y;
        
        // Calculate distance to current waypoint
        int16_t dx = target_x - car_x;
        int16_t dy = target_y - car_y;
        uint32_t dist_sq = (int32_t)dx * dx + (int32_t)dy * dy;
        
        // If within 40 pixels, move to NEXT waypoint early
        if (dist_sq < (40 * 40)) {
            ai->current_waypoint = (ai->current_waypoint + 1) % NUM_WAYPOINTS;
            // Generate new random offset
            ai->offset_x = (rand() % 20) - 10;
            ai->offset_y = (rand() % 20) - 10;
            
            // Update target to new waypoint
            target_x = waypoints[ai->current_waypoint].x + ai->offset_x;
            target_y = waypoints[ai->current_waypoint].y + ai->offset_y;
            
            // Recalculate distance
            dx = target_x - car_x;
            dy = target_y - car_y;
        }
        
        // Use cross product to determine turn direction (replaces atan2)
        // Forward vector from car angle
        int8_t forward_x = -SIN_LUT[ai->car.angle];  // cos component
        int8_t forward_y = -SIN_LUT[(ai->car.angle + 64) & 0xFF];  // sin component
        
        // Target vector (already calculated as dx, dy)
        
        // Cross product: (f_x * t_y) - (f_y * t_x)
        int32_t cross = ((int32_t)forward_x * dy) - ((int32_t)forward_y * dx);
        
        // Determine target angle based on cross product
        uint8_t target_angle;
        if (cross > 0) {
            // Turn right (clockwise)
            target_angle = ai->car.angle + 1;
        } else if (cross < 0) {
            // Turn left (counter-clockwise)
            target_angle = ai->car.angle - 1;
        } else {
            // Straight ahead
            target_angle = ai->car.angle;
        }
        
        // Steer toward target using wrap-around logic
        ai_steer_toward(ai, target_angle);
        
        // Calculate absolute angle difference for throttle control
        uint8_t angle_diff = target_angle - ai->car.angle;
        if (angle_diff > 128) angle_diff = 256 - angle_diff;
        
        // AI brake logic - adjust thrust based on turn sharpness
        if (angle_diff > 32) {
            // Sharp turn! Use reduced acceleration
            ai_apply_thrust(ai, AI_REDUCED_ACCEL);
        } else {
            // Straight line! Full speed
            ai_apply_thrust(ai, AI_MAX_ACCEL);
        }
        
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
