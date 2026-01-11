#include <stdint.h>
#include <stdbool.h>
#include "racelogic.h"
#include "hud.h"
#include "player.h"
#include "ai.h"

GameState current_state = STATE_TITLE;
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

void reset_race(void) {
    init_player(); // Resets car x,y, angle, laps, checkpoints
    init_ai();     // Resets all 3 AI cars to grid
    
    // Clear HUD
    for (uint8_t y=0; y<MESSAGE_HEIGHT; y++) {
        hud_print(0, y, "                                        ", 0, 0);
    }
    
    current_state = STATE_TITLE;
    countdown_active = false;
}