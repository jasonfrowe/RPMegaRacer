#include <string.h>
#include <rp6502.h>
#include "constants.h"
#include <stdint.h>
#include <stdio.h>
#include "hud.h"
#include "input.h"
#include "racelogic.h"
#include "player.h"

char message[MESSAGE_LENGTH + 1]; // +1 for null terminator

void hud_print(uint8_t x, uint8_t y, const char* str, uint8_t fg, uint8_t bg) {
    if (y >= MESSAGE_HEIGHT || x >= MESSAGE_WIDTH) return;

    // Calculate XRAM address for the specific character cell
    // (y * width + x) * 3 bytes per char
    RIA.addr0 = text_message_addr + (y * MESSAGE_WIDTH + x) * 3;
    RIA.step0 = 1;

    while (*str) {
        RIA.rw0 = *str++; // Glyph
        RIA.rw0 = fg;     // Foreground
        RIA.rw0 = bg;     // Background
        
        // Safety: don't over-run the row
        if (++x >= MESSAGE_WIDTH) break;
    }
}

void hud_refresh_stats(uint8_t lap, uint16_t speed) {
    char buf[10];
    
    // 1. Update Lap Display (Top Left)
    // LAP: 1/5
    sprintf(buf, "LAP:%d/5", lap + 1);
    hud_print(HUD_COL_LAPS, HUD_ROW, buf, HUD_COL_WHITE, HUD_COL_BG);

    // 2. Update Speed Display (Top Right)
    // Logic: speed is 8.8, so high byte is roughly MPH
    sprintf(buf, "%3d MPH", speed >> 2);
    hud_print(HUD_COL_TIME, HUD_ROW, buf, 11, HUD_COL_BG); // Light Blue for speed
}

void update_countdown_display(uint16_t delay) {

    if (delay > 480) {
        hud_print(16, 15, " PREPARE! ", HUD_COL_WHITE, HUD_COL_BG);
    } else if (delay > 420) {
        hud_print(16, 15, "    3     ", 12, HUD_COL_BG); // Red 3
    } else if (delay > 360) {
        hud_print(16, 15, "    2     ", 14, HUD_COL_BG); // Yellow 2
    } else if (delay > 300) {
        hud_print(16, 15, "    1     ", 15, HUD_COL_BG); // White 1
    } else if (delay > 240) {
        hud_print(16, 15, "   GO!!   ", 10, HUD_COL_BG); // Green GO
    } else if (delay <= 240) {
        hud_print(16, 15, "          ", 0, 0);         // Clear row 15
    }
}

void update_title_screen(void) {
    // Flash "PRESS FIRE" using vsync bits
    if (RIA.vsync & 0x20) {
        hud_print(10, 15, " PRESS FIRE TO START ", 15, 0);
    } else {
        hud_print(10, 15, "                     ", 0, 0);
    }
    
    hud_print(7, 12, " *** MEGA RACER RP6502 *** ", 11, 0);

    if (is_action_pressed(0, ACTION_FIRE)) {
        // Clear title text
        hud_print(10, 15, "                     ", 0, 0);
        hud_print(7, 12 ,  "                           ", 0, 0);
        current_state = STATE_COUNTDOWN;
        state_timer = COUNTDOWN_TOTAL_TIME; 
    }
}

uint8_t race_winner = 0;

void update_finished_screen(void) {
    // Access the winner defined in main
    extern uint8_t race_winner;

    hud_print(13, 12, " RACE FINISHED ", HUD_COL_CYAN, HUD_COL_BG);
    
    if (race_winner == 0) {
        // Player Victory
        hud_print(15, 14, " YOU WON! ", HUD_COL_GREEN, HUD_COL_BG);
        hud_print(9, 16, " CHAMPION OF THE TRACK ", HUD_COL_YELLOW, HUD_COL_BG);
    } else {
        // AI Victory
        char ai_msg[] = " AI 0 WON! ";
        ai_msg[4] = race_winner + '0'; // Convert ID to character '1', '2', or '3'
        
        hud_print(15, 14, ai_msg, HUD_COL_RED, HUD_COL_BG);
        hud_print(9, 16, " BETTER LUCK NEXT TIME ", HUD_COL_GREY, HUD_COL_BG);
    }

    if (state_timer > 0) {
        state_timer--;
    } else {
        // Blink the reset prompt
        if (RIA.vsync & 0x10) {
            hud_print(10, 18, " PRESS START TO RESET ", HUD_COL_WHITE, HUD_COL_BG);
        } else {
            hud_print(10, 18, "                      ", 0, 0);
        }

        if (is_action_pressed(0, ACTION_PAUSE)) { 
            reset_race(); 
        }
    }
}

