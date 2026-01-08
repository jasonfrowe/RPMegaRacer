#include <rp6502.h>
#include <stdint.h>
#include "player.h"
#include "constants.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "track.h"

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

void init_player(void) {
    // Initialize player car position and velocity
    car.x = 260L << 8;
    car.y = 60L << 8;
    car.vel_x = 0;
    car.vel_y = 0;
    car.angle = 64; // Facing Left (90 degrees)  0=Up, 64=Left, 128=Down, 192=Right

}

void update_player(Car *p) {
    // 1. Handle Rotation
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
    if (p->vel_x < 4 && p->vel_x > -4) p->vel_x = 0;
    if (p->vel_y < 4 && p->vel_y > -4) p->vel_y = 0;


    // 4. Calculate car corners in world pixels
    uint16_t x = p->x >> 8;
    uint16_t y = p->y >> 8;
    
    // We check a slightly smaller hitbox (e.g., 5 pixels out from center) 
    // so the car can "overlap" edges slightly without crashing.
    uint8_t tl = get_terrain_at(x - 5, y - 5);
    uint8_t tr = get_terrain_at(x + 5, y - 5);
    uint8_t bl = get_terrain_at(x - 5, y + 5);
    uint8_t br = get_terrain_at(x + 5, y + 5);

    // 5. Handle Wall Collisions (Bounce)
    if (tl == TERRAIN_WALL || tr == TERRAIN_WALL || bl == TERRAIN_WALL || br == TERRAIN_WALL) {
        p->vel_x = -(p->vel_x >> 1); // Reverse and half speed
        p->vel_y = -(p->vel_y >> 1);
        
        // Push car out of wall slightly to prevent sticking
        p->x += p->vel_x; 
        p->y += p->vel_y;

        // SFX: Trigger OPL2 "Thud"
        // play_opl2_sfx(SFX_CRASH);
    } 
    // 6. Handle Grass (Slowdown)
    else if (tl == TERRAIN_GRASS || tr == TERRAIN_GRASS || bl == TERRAIN_GRASS || br == TERRAIN_GRASS) {
        // Apply much heavier friction on grass
        p->vel_x -= (p->vel_x >> 3); 
        p->vel_y -= (p->vel_y >> 3);
    }


    // 7. Apply Velocity to Position
    // Position is 24.8, Velocity is 8.8. They add together perfectly.
    p->x += p->vel_x;
    p->y += p->vel_y;

    // 8. Clamp to world map bounds (512x384)
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

void check_collisions(Car *p) {
    int16_t world_x = p->x >> 8;
    int16_t world_y = p->y >> 8;

    // Get the rotation components (reuse these from your update loop!)
    int8_t s = SIN_LUT[p->angle];
    int8_t c = SIN_LUT[(p->angle + 64) & 0xFF];

    // --- 1. THE NOSE (6 pixels forward) ---
    // In your coordinate system: Forward X = sin, Forward Y = -cos
    int16_t nose_x = world_x + ((int16_t)s * 6 >> 7);
    int16_t nose_y = world_y - ((int16_t)c * 6 >> 7);

    // --- 2. REAR LEFT (-3 width, -5 length) ---
    // We calculate this by combining the Forward vector and the "Side" vector
    // Side Vector is just Forward rotated 90 degrees (c, s)
    int16_t rl_x = world_x - ((int16_t)s * 5 >> 7) - ((int16_t)c * 3 >> 7);
    int16_t rl_y = world_y + ((int16_t)c * 5 >> 7) - ((int16_t)s * 3 >> 7);

    // --- 3. REAR RIGHT (+3 width, -5 length) ---
    int16_t rr_x = world_x - ((int16_t)s * 5 >> 7) + ((int16_t)c * 3 >> 7);
    int16_t rr_y = world_y + ((int16_t)c * 5 >> 7) + ((int16_t)s * 3 >> 7);

    // Now check terrain at these three points
    uint8_t terrain_nose = get_terrain_at(nose_x, nose_y);
    uint8_t terrain_rl   = get_terrain_at(rl_x, rl_y);
    uint8_t terrain_rr   = get_terrain_at(rr_x, rr_y);

    // --- LOGIC ---
    if (terrain_nose == TERRAIN_WALL) {
        // Hit a wall head-on: Push back and bounce
        // Move car back in opposite direction of velocity to get out of wall
        int16_t push_dist = 8; // Push back 8 pixels
        if (p->vel_x != 0 || p->vel_y != 0) {
            // Calculate normalized push direction (opposite of velocity)
            int16_t mag = (abs(p->vel_x) + abs(p->vel_y));
            if (mag > 0) {
                p->x -= ((int32_t)p->vel_x * push_dist * 256) / mag;
                p->y -= ((int32_t)p->vel_y * push_dist * 256) / mag;
            }
        }
        // Reverse and dampen velocity
        p->vel_x = -(p->vel_x >> 1);
        p->vel_y = -(p->vel_y >> 1);
        // play_opl2_sfx(SFX_BUMP);
    } 
    
    // Slowdown logic: if any point is on grass, the car is slowed
    if (terrain_nose == TERRAIN_GRASS || terrain_rl == TERRAIN_GRASS || terrain_rr == TERRAIN_GRASS) {
        p->vel_x -= (p->vel_x >> 4);
        p->vel_y -= (p->vel_y >> 4);
    }
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