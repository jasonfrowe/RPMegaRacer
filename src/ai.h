#ifndef AI_H
#define AI_H

#include <stdint.h>
#include "player.h"

#define NUM_AI_CARS 3
#define NUM_WAYPOINTS 12
#define WAYPOINT_REACH_RADIUS 40
#define WAYPOINT_LOOKAHEAD 10

// Waypoint structure
typedef struct {
    int16_t x;
    int16_t y;
} Waypoint;

// AI car state
typedef struct {
    Car car;                    // Physics (position, velocity, angle)
    uint8_t current_waypoint;   // Current target waypoint index
    int8_t offset_x;           // Random offset from waypoint
    int8_t offset_y;           // Random offset from waypoint
    uint8_t sprite_index;      // Which sprite config (1-3)
    uint16_t startup_delay;    // Delay before AI starts driving (frames)
    uint8_t stuck_timer;       // Frames spent stuck (for recovery)
} AICar;

// External declarations
extern AICar ai_cars[NUM_AI_CARS];
extern Waypoint waypoints[NUM_WAYPOINTS];

// Functions
void init_ai(void);
void update_ai(void);
void draw_ai_cars(void);

#endif // AI_H
