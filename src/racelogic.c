#include <stdint.h>
#include <stdbool.h>
#include "racelogic.h"
#include "hud.h"

RaceState current_state = STATE_COUNTDOWN;
uint16_t state_timer = COUNTDOWN_TOTAL_TIME; // 4 seconds total (3, 2, 1, GO)
bool countdown_active = false;  

void update_race_logic(void) {
    if (current_state == STATE_COUNTDOWN) {
        if (state_timer > 0) {
            state_timer--;
            
            // // Visual Updates (every 60 frames / 1 second)
            // if (state_timer > 120)      hud_print(18, 1, " 3 ", 15, 0);
            // else if (state_timer > 60) hud_print(18, 1, " 2 ", 15, 0);
            // else if (state_timer >  0)  hud_print(18, 1, " 1 ", 15, 0);
            // else if (state_timer == 0)   hud_print(18, 1, "GO!", 10, 0); // Green text
        } else {
            current_state = STATE_RACING;
            // hud_print(18, 1, "   ", 0, 0); // Clear "GO!"
        }
    }
}