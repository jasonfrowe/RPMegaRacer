#include <rp6502.h>
#include <stdint.h>
#include "player.h"
#include "constants.h"
#include "input.h"
#include <stdlib.h>
#include "track.h"
#include "sound.h"

// External Sin table
extern const int8_t SIN_LUT[256];
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

const int16_t TX_LUT[256] = {
       16,    80,   128,   176,   224,   272,   336,   384,   448,   512,   560,   624,   688,   752,   816,   880,
      944,  1008,  1088,  1152,  1200,  1280,  1360,  1408,  1488,  1568,  1616,  1696,  1760,  1824,  1904,  1984,
     2048,  2112,  2192,  2272,  2336,  2400,  2480,  2528,  2608,  2688,  2736,  2816,  2896,  2944,  3008,  3088,
     3152,  3216,  3280,  3344,  3408,  3472,  3536,  3584,  3648,  3712,  3760,  3824,  3872,  3920,  3968,  4016,
     4080,  4112,  4160,  4208,  4256,  4304,  4336,  4384,  4416,  4448,  4496,  4528,  4560,  4592,  4624,  4656,
     4688,  4720,  4736,  4768,  4784,  4800,  4816,  4832,  4848,  4864,  4880,  4896,  4896,  4896,  4912,  4896,
     4896,  4896,  4912,  4896,  4896,  4896,  4880,  4864,  4848,  4832,  4816,  4800,  4784,  4768,  4736,  4720,
     4688,  4656,  4624,  4592,  4560,  4528,  4496,  4448,  4416,  4384,  4336,  4304,  4256,  4208,  4160,  4112,
     4080,  4016,  3968,  3920,  3872,  3824,  3760,  3712,  3648,  3584,  3536,  3472,  3408,  3344,  3280,  3216,
     3152,  3088,  3008,  2944,  2896,  2816,  2736,  2688,  2608,  2528,  2480,  2400,  2336,  2272,  2192,  2112,
     2048,  1984,  1904,  1824,  1760,  1696,  1616,  1568,  1488,  1408,  1360,  1280,  1200,  1152,  1088,  1008,
      944,   880,   816,   752,   688,   624,   560,   512,   448,   384,   336,   272,   224,   176,   128,    80,
       16,   -16,   -64,  -112,  -160,  -208,  -240,  -288,  -320,  -352,  -400,  -432,  -464,  -496,  -528,  -560,
     -592,  -624,  -640,  -672,  -688,  -704,  -720,  -736,  -752,  -768,  -784,  -800,  -800,  -800,  -816,  -800,
     -800,  -800,  -816,  -800,  -800,  -800,  -784,  -768,  -752,  -736,  -720,  -704,  -688,  -672,  -640,  -624,
     -592,  -560,  -528,  -496,  -464,  -432,  -400,  -352,  -320,  -288,  -240,  -208,  -160,  -112,   -64,   -16,
};

const int16_t TY_LUT[256] = {
       16,   -16,   -64,  -112,  -160,  -208,  -240,  -288,  -320,  -352,  -400,  -432,  -464,  -496,  -528,  -560,
     -592,  -624,  -640,  -672,  -688,  -704,  -720,  -736,  -752,  -768,  -784,  -800,  -800,  -800,  -816,  -800,
     -800,  -800,  -816,  -800,  -800,  -800,  -784,  -768,  -752,  -736,  -720,  -704,  -688,  -672,  -640,  -624,
     -592,  -560,  -528,  -496,  -464,  -432,  -400,  -352,  -320,  -288,  -240,  -208,  -160,  -112,   -64,   -16,
       16,    80,   128,   176,   224,   272,   336,   384,   448,   512,   560,   624,   688,   752,   816,   880,
      944,  1008,  1088,  1152,  1200,  1280,  1360,  1408,  1488,  1568,  1616,  1696,  1760,  1824,  1904,  1984,
     2048,  2112,  2192,  2272,  2336,  2400,  2480,  2528,  2608,  2688,  2736,  2816,  2896,  2944,  3008,  3088,
     3152,  3216,  3280,  3344,  3408,  3472,  3536,  3584,  3648,  3712,  3760,  3824,  3872,  3920,  3968,  4016,
     4080,  4112,  4160,  4208,  4256,  4304,  4336,  4384,  4416,  4448,  4496,  4528,  4560,  4592,  4624,  4656,
     4688,  4720,  4736,  4768,  4784,  4800,  4816,  4832,  4848,  4864,  4880,  4896,  4896,  4896,  4912,  4896,
     4896,  4896,  4912,  4896,  4896,  4896,  4880,  4864,  4848,  4832,  4816,  4800,  4784,  4768,  4736,  4720,
     4688,  4656,  4624,  4592,  4560,  4528,  4496,  4448,  4416,  4384,  4336,  4304,  4256,  4208,  4160,  4112,
     4080,  4016,  3968,  3920,  3872,  3824,  3760,  3712,  3648,  3584,  3536,  3472,  3408,  3344,  3280,  3216,
     3152,  3088,  3008,  2944,  2896,  2816,  2736,  2688,  2608,  2528,  2480,  2400,  2336,  2272,  2192,  2112,
     2048,  1984,  1904,  1824,  1760,  1696,  1616,  1568,  1488,  1408,  1360,  1280,  1200,  1152,  1088,  1008,
      944,   880,   816,   752,   688,   624,   560,   512,   448,   384,   336,   272,   224,   176,   128,    80,
};

void init_player(void) {
    // Initialize player car position and velocity
    // Starting position: back of grid, left of starting line
    car.x = 245L << 8;  // Left of starting line at X=255
    car.y = 70L << 8;   // Back position in track width (Y=33-86)
    car.vel_x = 0;
    car.vel_y = 0;
    car.angle = 64; // Facing Left (West, 0=Up, 64=Left, 128=Down)

}

uint8_t rebound_timer = 0;

// Tuning
#define BOUNCE_IMPULSE 0x080 
#define PUSH_OUT_DIST  0x060 
#define REBOUND_STUN   4     
#define HBOX           4     // 8x8 detection area

// OPTIMIZED: Checks 4 corners using 16-bit pixel coordinates
// This replaces the 49-lookup nested loop
static uint8_t is_colliding_fast(int16_t px, int16_t py) {
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

void update_player(Car *p) {
    // 1. Rotation
    if (is_action_pressed(0, ACTION_ROTATE_LEFT)) p->angle += TURN_SPEED;
    if (is_action_pressed(0, ACTION_ROTATE_RIGHT)) p->angle -= TURN_SPEED;

    int8_t s = SIN_LUT[p->angle];
    int8_t c = SIN_LUT[(p->angle + 64) & 0xFF];

    // 2. Thrust & Friction
    if (rebound_timer > 0) {
        rebound_timer--;
    } else {
        if (is_action_pressed(0, ACTION_FIRE)) {
            p->vel_x -= (int16_t)s >> THRUST_SCALER;
            p->vel_y -= (int16_t)c >> THRUST_SCALER;
        }
        if (is_action_pressed(0, ACTION_SUPER_FIRE)) {
            p->vel_x += (int16_t)s >> (THRUST_SCALER + 1);
            p->vel_y += (int16_t)c >> (THRUST_SCALER + 1);
        }
    }

    int16_t dvx = (p->vel_x >> FRICTION_SHIFT);
    int16_t dvy = (p->vel_y >> FRICTION_SHIFT);
    if (dvx == 0 && p->vel_x != 0) dvx = (p->vel_x > 0) ? 1 : -1;
    if (dvy == 0 && p->vel_y != 0) dvy = (p->vel_y > 0) ? 1 : -1;
    p->vel_x -= dvx; p->vel_y -= dvy;

    // --- 3. OPTIMIZED COLLISION ---
    // Convert 32-bit position to 16-bit pixels ONCE
    int16_t cur_px_x = (int16_t)(p->x >> 8);
    int16_t cur_px_y = (int16_t)(p->y >> 8);

    // Try X
    if (p->vel_x != 0) {
        int16_t next_px_x = (int16_t)((p->x + p->vel_x) >> 8);
        if (is_colliding_fast(next_px_x, cur_px_y)) {
            p->vel_x = (p->vel_x > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
            p->x += (p->vel_x > 0 ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
            rebound_timer = REBOUND_STUN;
        } else {
            p->x += p->vel_x;
            cur_px_x = next_px_x; // Update local pixel for Y-check
        }
    }

    // Try Y
    if (p->vel_y != 0) {
        int16_t next_px_y = (int16_t)((p->y + p->vel_y) >> 8);
        if (is_colliding_fast(cur_px_x, next_px_y)) {
            p->vel_y = (p->vel_y > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
            p->y += (p->vel_y > 0 ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
            rebound_timer = REBOUND_STUN;
        } else {
            p->y += p->vel_y;
        }
    }

    // Terrain speed check (only center point)
    if (get_terrain_at(cur_px_x + 8, (int16_t)(p->y >> 8) + 8) == TERRAIN_GRASS) {
        p->vel_x -= (p->vel_x >> 3);
        p->vel_y -= (p->vel_y >> 3);
    }

    update_engine_sound((uint16_t)(abs(p->vel_x) + abs(p->vel_y)));
}

// OPTIMIZED: Direct XRAM writes instead of struct_set overhead
void draw_player(Car *p, int16_t screen_x, int16_t screen_y) {
    int16_t s = (int16_t)SIN_LUT[p->angle] << 1;
    int16_t c = (int16_t)SIN_LUT[(p->angle + 64) & 0xFF] << 1;
    // int16_t tx = 8 * (256 - c + s);
    // int16_t ty = 8 * (256 - c - s);

    // Pivot Offsets from the NEW LUTs
    uint8_t ang = p->angle;
    int16_t tx = TX_LUT[ang];
    int16_t ty = TY_LUT[ang];

    RIA.addr0 = REDRACER_CONFIG; 
    RIA.step0 = 1;

    // Fast sequential write to the vga_mode4_asprite_t structure
    RIA.rw0 = c & 0xFF;        RIA.rw0 = c >> 8;        // transform[0] (sx)
    RIA.rw0 = (-s) & 0xFF;     RIA.rw0 = (-s) >> 8;     // transform[1] (shy)
    RIA.rw0 = tx & 0xFF;       RIA.rw0 = tx >> 8;       // transform[2] (tx)
    RIA.rw0 = s & 0xFF;        RIA.rw0 = s >> 8;        // transform[3] (shx)
    RIA.rw0 = c & 0xFF;        RIA.rw0 = c >> 8;        // transform[4] (sy)
    RIA.rw0 = ty & 0xFF;       RIA.rw0 = ty >> 8;       // transform[5] (ty)
    RIA.rw0 = screen_x & 0xFF; RIA.rw0 = screen_x >> 8; // x
    RIA.rw0 = screen_y & 0xFF; RIA.rw0 = screen_y >> 8; // y
}

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

void update_camera(Car *p) {
    int16_t car_px_x = (int16_t)(p->x >> 8);
    int16_t car_px_y = (int16_t)(p->y >> 8);
    int16_t target_x = 160 - car_px_x;
    int16_t target_y = 120 - car_px_y;

    if (target_x > 0) target_x = 0;
    if (target_x < -192) target_x = -192;
    if (target_y > 0) target_y = 0;
    if (target_y < -144) target_y = -144;

    // Use shadow registers in main.c instead of writing XRAM here twice
    extern int16_t next_scroll_x, next_scroll_y;
    next_scroll_x = target_x;
    next_scroll_y = target_y;
}