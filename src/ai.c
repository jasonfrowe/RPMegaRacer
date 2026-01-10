#include "ai.h"
#include "constants.h"
#include "track.h"
#include "player.h"
#include <rp6502.h>
#include <stdlib.h>

#define AI_TURN_SPEED 3
#define AI_MAX_THRUST_SHIFT 3      
#define AI_REDUCED_THRUST_SHIFT 4  

#define BOUNCE_IMPULSE 0x080  
#define PUSH_OUT_DIST  0x060  
#define REBOUND_STUN   4      
#define HBOX           4

extern const int8_t SIN_LUT[256];

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

        // --- 1. RECOVERY STATE ---
        if (ai->recovery_timer > 0) {
            ai->recovery_timer--;
            int8_t s = SIN_LUT[ai->car.angle];
            int8_t c = SIN_LUT[(ai->car.angle + 64) & 0xFF];
            ai->car.vel_x += (int16_t)s >> 4;  
            ai->car.vel_y += (int16_t)c >> 4;
            ai->car.angle += 4 * ai->recovery_turn_dir;
            ai->car.x += ai->car.vel_x;
            ai->car.y += ai->car.vel_y;
            continue; 
        }

        if (ai->rebound_timer > 0) ai->rebound_timer--;

        // --- 2. STUCK DETECTION ---
        ai->stuck_timer++;
        if (ai->stuck_timer >= 30) {
            ai->stuck_timer = 0;
            int16_t cur_x = (int16_t)(ai->car.x >> 8);
            int16_t cur_y = (int16_t)(ai->car.y >> 8);
            if (abs(cur_x - ai->last_recorded_x) < 3 && abs(cur_y - ai->last_recorded_y) < 3) {
                ai->recovery_timer = 45;
                ai->recovery_turn_dir = (rand() & 1) ? 1 : -1;
            }
            ai->last_recorded_x = cur_x;
            ai->last_recorded_y = cur_y;
        }

        // --- 3. WAYPOINTS & THRUST ---
        int16_t tx = waypoints[ai->current_waypoint].x + ai->offset_x;
        int16_t ty = waypoints[ai->current_waypoint].y + ai->offset_y;
        int16_t dx = tx - (int16_t)(ai->car.x >> 8);
        int16_t dy = ty - (int16_t)(ai->car.y >> 8);

        int16_t dist = abs(dx) + abs(dy);
        if (dist < 50) { // 50 is a rough approximation of a 40px radius
            ai->current_waypoint = (ai->current_waypoint + 1) % NUM_WAYPOINTS;
        }
        
        uint8_t target_angle = (192 - atan2_8(dy, dx)) & 0xFF;
        // ai_steer inline for speed
        uint8_t diff = target_angle - ai->car.angle;
        if (diff != 0) {
            if (diff < 128) ai->car.angle += AI_TURN_SPEED;
            else ai->car.angle -= AI_TURN_SPEED;
        }

        if (ai->rebound_timer == 0) {
            int8_t s = SIN_LUT[ai->car.angle];
            int8_t c = SIN_LUT[(ai->car.angle + 64) & 0xFF];
            uint8_t shift = (abs((int8_t)(target_angle - ai->car.angle)) > 32) ? AI_REDUCED_THRUST_SHIFT : AI_MAX_THRUST_SHIFT;
            ai->car.vel_x -= (int16_t)s >> shift;
            ai->car.vel_y -= (int16_t)c >> shift;
        }

        // --- 4. FRICTION ---
        int16_t dvx = (ai->car.vel_x >> FRICTION_SHIFT);
        int16_t dvy = (ai->car.vel_y >> FRICTION_SHIFT);
        if (dvx == 0 && ai->car.vel_x != 0) dvx = (ai->car.vel_x > 0) ? 1 : -1;
        if (dvy == 0 && ai->car.vel_y != 0) dvy = (ai->car.vel_y > 0) ? 1 : -1;
        ai->car.vel_x -= dvx; ai->car.vel_y -= dvy;

        // --- 5. OPTIMIZED AXIS MOVEMENT ---
        int16_t px = (int16_t)(ai->car.x >> 8);
        int16_t py = (int16_t)(ai->car.y >> 8);

        if (ai->car.vel_x != 0) {
            int16_t nx = (int16_t)((ai->car.x + ai->car.vel_x) >> 8);
            if (is_colliding_ai(nx, py)) {
                ai->car.vel_x = (ai->car.vel_x > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
                ai->car.x += (ai->car.vel_x > 0 ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
                ai->rebound_timer = REBOUND_STUN;
            } else {
                ai->car.x += ai->car.vel_x;
                px = nx;
            }
        }
        if (ai->car.vel_y != 0) {
            int16_t ny = (int16_t)((ai->car.y + ai->car.vel_y) >> 8);
            if (is_colliding_ai(px, ny)) {
                ai->car.vel_y = (ai->car.vel_y > 0) ? -BOUNCE_IMPULSE : BOUNCE_IMPULSE;
                ai->car.y += (ai->car.vel_y > 0 ? PUSH_OUT_DIST : -PUSH_OUT_DIST);
                ai->rebound_timer = REBOUND_STUN;
            } else {
                ai->car.y += ai->car.vel_y;
            }
        }

        // Clamp world bounds
        if (ai->car.x < 0x800) ai->car.x = 0x800;
        if (ai->car.x > 131072L - 0x800) ai->car.x = 131072L - 0x800;
        if (ai->car.y < 0x800) ai->car.y = 0x800;
        if (ai->car.y > 98304L - 0x800) ai->car.y = 98304L - 0x800;
    }
}

// Optimized Sequential Writes for AI
void draw_ai_cars(int16_t scroll_x, int16_t scroll_y) {
    for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
        AICar *ai = &ai_cars[i];
        
        int16_t s = (int16_t)SIN_LUT[ai->car.angle] << 1;
        int16_t c = (int16_t)SIN_LUT[(ai->car.angle + 64) & 0xFF] << 1;
        
        uint8_t ang = ai->car.angle;
        int16_t tx = TX_LUT[ang];
        int16_t ty = TY_LUT[ang];
        
        int16_t sx = (int16_t)(ai->car.x >> 8) + scroll_x;
        int16_t sy = (int16_t)(ai->car.y >> 8) + scroll_y;

        // Correct Struct Layout per Docs: transform[6], x, y, ptr, log, opacity
        RIA.addr0 = REDRACER_CONFIG + (sizeof(vga_mode4_asprite_t) * ai->sprite_index);
        RIA.step0 = 1;

        // Transform [0..5]
        RIA.rw0 = c & 0xFF;    RIA.rw0 = c >> 8;    // SX
        RIA.rw0 = (-s) & 0xFF; RIA.rw0 = (-s) >> 8; // SHY
        RIA.rw0 = tx & 0xFF;   RIA.rw0 = tx >> 8;   // TX
        RIA.rw0 = s & 0xFF;    RIA.rw0 = s >> 8;    // SHX
        RIA.rw0 = c & 0xFF;    RIA.rw0 = c >> 8;    // SY
        RIA.rw0 = ty & 0xFF;   RIA.rw0 = ty >> 8;   // TY
        // Position
        RIA.rw0 = sx & 0xFF;   RIA.rw0 = sx >> 8;
        RIA.rw0 = sy & 0xFF;   RIA.rw0 = sy >> 8;
        // ptr, log_size, opacity are usually already set in init_ai and don't need frame-by-frame updates,
        // but for safety in a sequential write, we stop here or continue if values changed.
    }
}