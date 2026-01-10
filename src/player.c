#include <rp6502.h>
#include <stdint.h>
#include "player.h"
#include "constants.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "track.h"
#include "sound.h"

// Starting position for the player's car
uint8_t startX = SCREEN_WIDTH / 2;
uint8_t startY = SCREEN_HEIGHT / 2; 

Car car = {0};

// Pre-calculated Sin table (scaled to 127)
const int8_t SIN_LUT[256] = {
   0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,  46,
  49,  51,  54,  57,  60,  63,  65,  68,  71,  73,  76,  78,  81,  83,  85,  88,
  90,  92,  94,  96,  98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100,  98,  96,  94,  92,
  90,  88,  85,  83,  81,  78,  76,  73,  71,  68,  65,  63,  60,  57,  54,  51,
  49,  46,  43,  40,  37,  34,  31,  28,  25,  22,  19,  16,  12,   9,   6,   3,
   0,  -2,  -5,  -8, -11, -15, -18, -21, -24, -27, -30, -33, -36, -39, -42, -45,
 -48, -50, -53, -56, -59, -62, -64, -67, -70, -72, -75, -77, -80, -82, -84, -87,
 -89, -91, -93, -95, -97, -99,-101,-103,-105,-106,-108,-110,-111,-112,-114,-115,
-116,-117,-119,-120,-121,-121,-122,-123,-124,-124,-125,-125,-125,-126,-126,-126,
-126,-126,-126,-126,-125,-125,-125,-124,-124,-123,-122,-121,-121,-120,-119,-117,
-116,-115,-114,-112,-111,-110,-108,-106,-105,-103,-101, -99, -97, -95, -93, -91,
 -89, -87, -84, -82, -80, -77, -75, -72, -70, -67, -64, -62, -59, -56, -53, -50,
 -48, -45, -42, -39, -36, -33, -30, -27, -24, -21, -18, -15, -11,  -8,  -5,  -2,
};
// Cosine is just Sine with a 90-degree (64 step) offset
// cos_val = SIN_LUT[(angle + 64) & 0xFF];

// Forward declaration (made non-static for AI to use)
uint8_t check_collision_at_pos(int32_t x, int32_t y, uint8_t angle);

void init_player(void) {
    // Initialize player car position and velocity
    // Starting position: back of grid, left of starting line
    car.x = 245L << 8;  // Left of starting line at X=255
    car.y = 70L << 8;   // Back position in track width (Y=33-86)
    car.vel_x = 0;
    car.vel_y = 0;
    car.angle = 64; // Facing Left (West, 0=Up, 64=Left, 128=Down)

}

void update_player(Car *p) {
    // 1. Handle Rotation (no collision check - box is axis-aligned)
    if (is_action_pressed(0, ACTION_ROTATE_LEFT)) {
        p->angle += TURN_SPEED;
    }
    if (is_action_pressed(0, ACTION_ROTATE_RIGHT)) {
        p->angle -= TURN_SPEED;
    }

    // 2. Derive Sine and Cosine from the single SIN_LUT
    // Standard: 0 = North. x = sin, y = -cos (because Y increases down)
    int8_t sin_val = SIN_LUT[p->angle];
    int8_t cos_val = SIN_LUT[(p->angle + 64) & 0xFF];

    // 3. Handle Thrust (Acceleration)
    if (is_action_pressed(0, ACTION_THRUST)) {
        // We add the thrust vector to the velocity
        // We scale the LUT values (-127 to 127) to fit our 8.8 fixed point
        p->vel_x -= (int16_t)sin_val >> THRUST_SCALER;
        p->vel_y -= (int16_t)cos_val >> THRUST_SCALER;
    }
    
    // Handle Reverse Thrust (backing up)
    if (is_action_pressed(0, ACTION_REVERSE_THRUST)) {
        // Opposite direction of thrust, slightly weaker
        p->vel_x += (int16_t)sin_val >> (THRUST_SCALER + 1);
        p->vel_y += (int16_t)cos_val >> (THRUST_SCALER + 1);
    }

    // 4. Apply Friction (Drag)
    // This reduces velocity slightly every frame to simulate momentum
    int16_t drag_x = (p->vel_x >> FRICTION_SHIFT);
    int16_t drag_y = (p->vel_y >> FRICTION_SHIFT);

    // If the proportional drag is 0 but the car is still moving, 
    // force it to slow down by 1 unit.
    if (drag_x == 0 && p->vel_x != 0) {
        drag_x = (p->vel_x > 0) ? 1 : -1;
    }
    if (drag_y == 0 && p->vel_y != 0) {
        drag_y = (p->vel_y > 0) ? 1 : -1;
    }

    p->vel_x -= drag_x;
    p->vel_y -= drag_y;

    // 3. Deadzone (Optional but recommended)
    // If velocity is extremely low, just kill it to prevent "micro-drifting"
    // if (p->vel_x < 4 && p->vel_x > -4) p->vel_x = 0;
    // if (p->vel_y < 4 && p->vel_y > -4) p->vel_y = 0;

    // 4. Apply Velocity to Position (with collision check)
    // Position is 24.8, Velocity is 8.8. They add together perfectly.
    int32_t new_x = p->x + p->vel_x;
    int32_t new_y = p->y + p->vel_y;
    
    // Simple collision: if destination collides, don't move there
    if (check_collision_at_pos(new_x, new_y, p->angle) == TERRAIN_WALL) {
        // Don't move - just bounce velocity
        p->vel_x = -(p->vel_x >> 2);
        p->vel_y = -(p->vel_y >> 2);
    } else {
        // No collision, apply movement
        p->x = new_x;
        p->y = new_y;
    }

    update_engine_sound((uint16_t)(abs(p->vel_x) + abs(p->vel_y)));

    // 5. Clamp to world map bounds (512x384)
    // 512 in 8.8 is 512 * 256 = 131072 (0x20000)
    // 384 in 8.8 is 384 * 256 = 98304 (0x18000)
    if (p->x < 0) p->x = 0;
    if (p->x > 131072L) p->x = 131072L;
    if (p->y < 0) p->y = 0;
    if (p->y > 98304L) p->y = 98304L;
}

void draw_player(Car *p, int16_t screen_x, int16_t screen_y) {
    uint16_t ptr = REDRACER_CONFIG;

    // 1. Get Sin/Cos and scale to your ~255 range (127 * 2)
    int16_t s = (int16_t)SIN_LUT[p->angle] << 1;
    int16_t c = (int16_t)SIN_LUT[(p->angle + 64) & 0xFF] << 1;

    // 2. Set Rotation Matrix (SX, SHY, SHX, SY)
    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[0],  c); // SX
    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[1], -s); // SHY
    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[3],  s); // SHX
    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[4],  c); // SY

    // 3. Reconciled Pivot Translation (Logic from t2_fix4)
    // We use 8 as the multiplier for a 16x16 sprite centered at 8,8
    // Note: 256 is the RIA 1.0 identity scale
    int16_t tx = 8 * (256 - c + s);
    int16_t ty = 8 * (256 - c - s);

    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[2], tx);
    xram0_struct_set(ptr, vga_mode4_asprite_t, transform[5], ty);

    // 4. Update Position (Whole pixels)
    // xram0_struct_set(ptr, vga_mode4_asprite_t, x_pos_px, (int16_t)(p->x >> 8));
    // xram0_struct_set(ptr, vga_mode4_asprite_t, y_pos_px, (int16_t)(p->y >> 8));

    // Set the sprite position to the calculated screen coordinates
    xram0_struct_set(ptr, vga_mode4_asprite_t, x_pos_px, screen_x);
    xram0_struct_set(ptr, vga_mode4_asprite_t, y_pos_px, screen_y);
}

// We use the sin/cos values scaled by the LUT (127)
// To get pixel offsets, we multiply by the distance and divide by 127 (shift 7)

// Helper function to check if any collision point hits a wall at given position/angle
// Uses a solid rectangular hitbox - checks every pixel, no gaps
// Made non-static so AI can use it
uint8_t check_collision_at_pos(int32_t x, int32_t y, uint8_t angle) {
    (void)angle;  // Ignore rotation - use axis-aligned box
    
    int16_t cx = (x >> 8) + 8;  // Center of sprite
    int16_t cy = (y >> 8) + 8;
    
    // Check a 6x6 square centered on the car (smaller than actual car for better feel)
    // This is a solid hitbox - check every pixel
    uint8_t found_grass = 0;
    
    for (int16_t dy = -3; dy <= 3; dy++) {
        for (int16_t dx = -3; dx <= 3; dx++) {
            uint8_t terrain = get_terrain_at(cx + dx, cy + dy);
            if (terrain == TERRAIN_WALL) {
                return TERRAIN_WALL;  // Any wall pixel = collision
            }
            if (terrain == TERRAIN_GRASS) {
                found_grass = 1;
            }
        }
    }
    
    return found_grass ? TERRAIN_GRASS : TERRAIN_ROAD;
}

void check_collisions(Car *p) {
    uint8_t terrain = check_collision_at_pos(p->x, p->y, p->angle);
    
    if (terrain == TERRAIN_GRASS) {
        // Slowdown on grass
        p->vel_x -= (p->vel_x >> 4);
        p->vel_y -= (p->vel_y >> 4);
    }
    // Wall collisions are now handled in update_player() before movement
}

void update_camera(Car *p) {
    int16_t target_x = (p->x >> 8) - 160; // 160 is half of 320
    int16_t target_y = (p->y >> 8) - 120; // 120 is half of 240

    // Clamp camera to map edges (Map 512x384 - Screen 320x240)
    if (target_x < 0) target_x = 0;
    if (target_x > 192) target_x = 192;
    if (target_y < 0) target_y = 0;
    if (target_y > 144) target_y = 144;

    // Update XRAM Config for the Tile Plane
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, target_x);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, target_y);
    
    // IMPORTANT: You must also subtract the camera offset when drawing the car!
    // screen_x = car_world_x - camera_x
    uint16_t screen_x = (p->x >> 8) - target_x;
    uint16_t screen_y = (p->y >> 8) - target_y;
    
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, x_pos_px, screen_x);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, y_pos_px, screen_y);
}