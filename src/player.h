#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#define FRICTION_SHIFT 5  // Higher = less friction (more "icy")
#define THRUST_SCALER  2  // Tuning: how fast the car accelerates
#define TURN_SPEED     4  // How many angle units to turn per frame

typedef struct {
    int16_t x;         // Position X (24.8 Fixed Point)
    int16_t y;         // Position Y (24.8 Fixed Point)
    int16_t vel_x;     // Velocity X (8.8 Fixed Point)
    int16_t vel_y;     // Velocity Y (8.8 Fixed Point)
    uint8_t angle;     // 0-255 (0=Up/North)
    // --- Lap System ---
    uint8_t laps;            // Count of completed laps
    uint8_t next_checkpoint; // 0=Finish, 1=CP1, 2=CP2, 3=CP3
    uint16_t total_progress; // Track progress for AI comparison
} Car;

extern uint8_t startX;
extern uint8_t startY;
extern Car car;

// This table maps 0-255 (angle) to -127 to 127 (signed fraction)
extern const int8_t SIN_LUT[256];
extern const int16_t TX_LUT[256];
extern const int16_t TY_LUT[256];

extern void init_player(void);
extern void update_player(Car *p);
extern void draw_player(Car *p, int16_t screen_x, int16_t screen_y);
extern void update_camera(Car *p);
extern void update_lap_logic(Car *p, bool is_player);
extern uint8_t is_colliding_fast(int16_t px, int16_t py);
extern void update_player_progress(void);

#endif // PLAYER_H