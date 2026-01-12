#include <stdint.h>
#include <stdbool.h>
#include "racelogic.h"
#include "hud.h"
#include "player.h"
#include "ai.h"


// Race time tracking    
uint8_t race_minutes = 0;
uint8_t race_seconds = 0;
uint8_t race_frames = 0; // Ticks from 0-59


GameState current_state = STATE_TITLE;
uint16_t state_timer = COUNTDOWN_TOTAL_TIME; // 4 seconds total (3, 2, 1, GO)
bool countdown_active = false;  

void update_race_logic(void) {
    if (state_timer > 0) {
        state_timer--;
        
        // Trigger the start of the race at the "GO!" mark (300)
        if (state_timer == 300) {
            current_state = STATE_RACING;
            // Clear the "Prepare" message area if necessary
            hud_print(16, 15, "          ", 0, 0); 
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

    car.current_waypoint = 1; // Looking for the first corner
    car.progress_steps = 1;   // Start line is step 0, next is 1
    
    for (int i=0; i<3; i++) {
        ai_cars[i].car.current_waypoint = 1;
        ai_cars[i].car.progress_steps = 1;
        ai_cars[i].base_speed_shift = 5;
        ai_cars[i].car.laps = 0;
    }

    // ... car resets ...
    race_minutes = 0;
    race_seconds = 0;
    race_frames = 0;
    
    // Clear the timer area on the HUD
    hud_print(16, 0, "00:00:00", HUD_COL_WHITE, HUD_COL_BG);

    // Reset winner
    race_winner = 0;
    car.laps = 0;

}

void hud_draw_timer(void) {
    char timer_str[10]; // "00:00:00"

    // Convert Minutes
    timer_str[0] = (race_minutes / 10) + '0';
    timer_str[1] = (race_minutes % 10) + '0';
    timer_str[2] = ':';
    
    // Convert Seconds
    timer_str[3] = (race_seconds / 10) + '0';
    timer_str[4] = (race_seconds % 10) + '0';
    timer_str[5] = ':';

    // Convert Frames to Centiseconds (approx: frames * 1.66)
    // For simplicity, let's just show raw frames 00-59 first
    timer_str[6] = (race_frames / 10) + '0';
    timer_str[7] = (race_frames % 10) + '0';
    timer_str[8] = '\0';

    // Print to the top-center of the HUD (Column 16)
    hud_print(16, 0, timer_str, HUD_COL_WHITE, HUD_COL_BG);
}

void update_race_timer(void) {
    race_frames++;
    if (race_frames >= 60) {
        race_frames = 0;
        race_seconds++;
        if (race_seconds >= 60) {
            race_seconds = 0;
            race_minutes++;
            if (race_minutes > 99) race_minutes = 99; // Cap at 99 mins
        }
    }
}

bool is_player_leading(void) {
    for (int i = 0; i < NUM_AI_CARS; i++) {
        if (ai_cars[i].car.progress_steps > car.progress_steps) {
            return false; // Someone is further ahead than the player
        }
    }
    return true;
}