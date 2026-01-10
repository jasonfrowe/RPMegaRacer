#include "ai.h"
#include "constants.h"
#include "track.h"
#include "player.h"
#include <rp6502.h>
#include <stdlib.h>

#define AI_TURN_SPEED 3
#define AI_MAX_THRUST_SHIFT 3      
#define AI_REDUCED_THRUST_SHIFT 4  

// Arcade Physics Constants (Matched to Player)
#define BOUNCE_IMPULSE 0x080  
#define PUSH_OUT_DIST  0x060  
#define REBOUND_STUN   4      

extern const int8_t SIN_LUT[256];
extern uint8_t check_collision_at_pos(int32_t x, int32_t y, uint8_t angle);

// Standard atan2 in 8-bit: 0=Right, 64=Down, 128=Left, 192=Up
static uint8_t atan2_8(int16_t dy, int16_t dx) {
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

static void ai_steer(AICar *ai, uint8_t target_angle) {
    uint8_t diff = target_angle - ai->car.angle;
    if (diff == 0) return;
    if (diff < 128) ai->car.angle += AI_TURN_SPEED;
    else ai->car.angle -= AI_TURN_SPEED;
}

static void ai_apply_thrust(AICar *ai, uint8_t thrust_scaler) {
    // Only thrust if not in rebound stun
    if (ai->rebound_timer > 0) return;

    int8_t sin_val = SIN_LUT[ai->car.angle];
    int8_t cos_val = SIN_LUT[(ai->car.angle + 64) & 0xFF];
    ai->car.vel_x -= (int16_t)sin_val >> thrust_scaler;
    ai->car.vel_y -= (int16_t)cos_val >> thrust_scaler;
}

AICar ai_cars[NUM_AI_CARS];

Waypoint waypoints[NUM_WAYPOINTS] = {
    {255, 60}, {120, 50}, {91, 84}, {60, 192}, {48, 255}, {88, 295}, 
    {247, 315}, {375, 328}, {431, 295}, {435, 175}, {445, 116}, {417, 80}
};

void init_ai(void) {
    int16_t start_positions[NUM_AI_CARS] = {40, 50, 60};  
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        ai_cars[i].car.x = 245L << 8;  
        ai_cars[i].car.y = ((int32_t)start_positions[i]) << 8;  
        ai_cars[i].car.vel_x = 0;
        ai_cars[i].car.vel_y = 0;
        ai_cars[i].car.angle = 64;  
        ai_cars[i].current_waypoint = 1;  
        ai_cars[i].offset_x = (rand() % 20) - 10;  
        ai_cars[i].offset_y = (rand() % 20) - 10;
        ai_cars[i].sprite_index = i + 1;  
        ai_cars[i].startup_delay = 600;  
        ai_cars[i].stuck_timer = 0;  
        ai_cars[i].rebound_timer = 0; // Initialize stun timer
        ai_cars[i].recovery_timer = 0;
        ai_cars[i].recovery_turn_dir = 1;
        ai_cars[i].last_recorded_x = 245;  
        ai_cars[i].last_recorded_y = start_positions[i];
    }
}

void update_ai(void) {
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        if (ai->startup_delay > 0) {
            ai->startup_delay--;
            continue;
        }

        // --- 1. RECOVERY STATE CHECK (Major Stuckness) ---
        if (ai->recovery_timer > 0) {
            ai->recovery_timer--;
            int8_t s = SIN_LUT[ai->car.angle];
            int8_t c = SIN_LUT[(ai->car.angle + 64) & 0xFF];
            ai->car.vel_x += (int16_t)s >> 4;  
            ai->car.vel_y += (int16_t)c >> 4;
            ai->car.angle += 4 * ai->recovery_turn_dir;
            
            // Movement for recovery is simpler (ignoring collision logic for exit)
            ai->car.x += ai->car.vel_x;
            ai->car.y += ai->car.vel_y;
            goto post_movement_ai; 
        }

        // Update Rebound Stun Timer
        if (ai->rebound_timer > 0) ai->rebound_timer--;

        // --- 2. STUCK DETECTION ---
        ai->stuck_timer++;
        if (ai->stuck_timer >= 30) {
            ai->stuck_timer = 0;
            int16_t cur_x = ai->car.x >> 8;
            int16_t cur_y = ai->car.y >> 8;
            int16_t dx = abs(cur_x - ai->last_recorded_x);
            int16_t dy = abs(cur_y - ai->last_recorded_y);
            if (dx < 3 && dy < 3) {
                ai->recovery_timer = 45;
                ai->recovery_turn_dir = (rand() & 1) ? 1 : -1;
            }
            ai->last_recorded_x = cur_x;
            ai->last_recorded_y = cur_y;
        }

        // --- 3. WAYPOINT & STEERING ---
        int16_t target_x = waypoints[ai->current_waypoint].x + ai->offset_x;
        int16_t target_y = waypoints[ai->current_waypoint].y + ai->offset_y;
        int16_t dx = target_x - (ai->car.x >> 8);
        int16_t dy = target_y - (ai->car.y >> 8);
        if (((int32_t)dx * dx + (int32_t)dy * dy) < (40 * 40)) {
            ai->current_waypoint = (ai->current_waypoint + 1) % NUM_WAYPOINTS;
        }
        
        uint8_t target_angle = (192 - atan2_8(dy, dx)) & 0xFF;
        ai_steer(ai, target_angle);
        
        uint8_t angle_diff = abs((int8_t)(target_angle - ai->car.angle));
        if (angle_diff > 32) ai_apply_thrust(ai, AI_REDUCED_THRUST_SHIFT);
        else ai_apply_thrust(ai, AI_MAX_THRUST_SHIFT);

        // --- 4. FRICTION ---
        int16_t drag_x = (ai->car.vel_x >> FRICTION_SHIFT);
        int16_t drag_y = (ai->car.vel_y >> FRICTION_SHIFT);
        if (drag_x == 0 && ai->car.vel_x != 0) drag_x = (ai->car.vel_x > 0) ? 1 : -1;
        if (drag_y == 0 && ai->car.vel_y != 0) drag_y = (ai->car.vel_y > 0) ? 1 : -1;
        ai->car.vel_x -= drag_x;
        ai->car.vel_y -= drag_y;

        // --- 5. COMPONENT-WISE MOVEMENT WITH BOUNCE/PUSH-OUT ---

        // X-Axis
        int32_t old_x = ai->car.x;
        ai->car.x += ai->car.vel_x;
        if (check_collision_at_pos(ai->car.x, ai->car.y, ai->car.angle) == TERRAIN_WALL) {
            ai->car.vel_x = (ai->car.vel_x > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
            ai->car.x = old_x + ((ai->car.vel_x > 0) ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
            ai->rebound_timer = REBOUND_STUN;
        }

        // Y-Axis
        int32_t old_y = ai->car.y;
        ai->car.y += ai->car.vel_y;
        if (check_collision_at_pos(ai->car.x, ai->car.y, ai->car.angle) == TERRAIN_WALL) {
            ai->car.vel_y = (ai->car.vel_y > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
            ai->car.y = old_y + ((ai->car.vel_y > 0) ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
            ai->rebound_timer = REBOUND_STUN;
        }

post_movement_ai:
        // Clamp to world bounds
        if (ai->car.x < 0x800) ai->car.x = 0x800;
        if (ai->car.x > 131072L - 0x800) ai->car.x = 131072L - 0x800;
        if (ai->car.y < 0x800) ai->car.y = 0x800;
        if (ai->car.y > 98304L - 0x800) ai->car.y = 98304L - 0x800;
    }
}


void draw_ai_cars(int16_t scroll_x, int16_t scroll_y) {
    extern const int8_t SIN_LUT[256];

    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        
        // Calculate the XRAM address for this specific car's config struct
        // Uses sprite_index (1, 2, 3) to offset from the Player's config at index 0
        uint16_t config_addr = REDRACER_CONFIG + (sizeof(vga_mode4_asprite_t) * ai->sprite_index);
        
        // Calculate screen position: (World Position) + (Map Origin Screen Offset)
        int16_t screen_x = (int16_t)(ai->car.x >> 8) + scroll_x;
        int16_t screen_y = (int16_t)(ai->car.y >> 8) + scroll_y;
        
        // --- 1. SET ROTATION MATRIX ---
        // Scale to 8.8 (1.0 = 256)
        int16_t s = (int16_t)SIN_LUT[ai->car.angle] << 1;
        int16_t c = (int16_t)SIN_LUT[(ai->car.angle + 64) & 0xFF] << 1;
        
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[0], c);  // SX
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[1], -s); // SHY
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[3], s);  // SHX
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[4], c);  // SY
        
        // --- 2. SET PIVOT TRANSLATION ---
        // Formula to rotate 16x16 around center (8,8)
        int16_t tx = 8 * (256 - c + s);
        int16_t ty = 8 * (256 - c - s);
        
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[2], tx);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[5], ty);
        
        // --- 3. SET SCREEN POSITION ---
        xram0_struct_set(config_addr, vga_mode4_asprite_t, x_pos_px, screen_x);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, y_pos_px, screen_y);
    }
}
