#ifndef RACELOGIC_H
#define RACELOGIC_H

#define COUNTDOWN_TOTAL_TIME 480 // 4 seconds at 120 FPS

typedef enum {
    STATE_TITLE,
    STATE_COUNTDOWN,
    STATE_RACING,
    STATE_FINISHED,
    STATE_GAMEOVER
} GameState;

extern void update_race_logic(void);
extern void reset_race(void);
extern void update_race_timer(void);
extern void hud_draw_timer(void);
extern bool is_player_leading(void);

extern GameState current_state;
extern uint16_t state_timer;
extern bool countdown_active;

#endif // RACELOGIC_H