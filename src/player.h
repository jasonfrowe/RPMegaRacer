#ifndef PLAYER_H
#define PLAYER_H

#define FRICTION_SHIFT 5  // Higher = less friction (more "icy")
#define THRUST_SCALER  3  // Tuning: how fast the car accelerates
#define TURN_SPEED     4  // How many angle units to turn per frame

typedef struct {
    int32_t x;         // Position X (24.8 Fixed Point)
    int32_t y;         // Position Y (24.8 Fixed Point)
    int16_t vel_x;     // Velocity X (8.8 Fixed Point)
    int16_t vel_y;     // Velocity Y (8.8 Fixed Point)
    uint8_t angle;     // 0-255 (0=Up/North)
} Car;

extern uint8_t startX;
extern uint8_t startY;
extern Car car;

// This table maps 0-255 (angle) to -127 to 127 (signed fraction)
extern const int8_t SIN_LUT[256];

extern void init_player(void);
extern void update_player(Car *p);
extern void draw_player(Car *p);

#endif // PLAYER_H