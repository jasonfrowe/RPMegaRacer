#ifndef RACELOGIC_H
#define RACELOGIC_H

#define COUNTDOWN_TOTAL_TIME 480 // 4 seconds at 120 FPS

typedef enum {
    STATE_COUNTDOWN,
    STATE_RACING,
    STATE_FINISHED
} RaceState;

extern void update_race_logic(void);

extern RaceState current_state;
extern uint16_t state_timer;
extern bool countdown_active;

#endif // RACELOGIC_H