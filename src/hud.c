#include <string.h>
#include <rp6502.h>
#include "constants.h"
#include <stdint.h>
#include <stdio.h>
#include "hud.h"

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
    // LAP: 1/3
    sprintf(buf, "LAP:%d/3", lap + 1);
    hud_print(HUD_COL_LAPS, HUD_ROW, buf, HUD_COL_WHITE, HUD_COL_BG);

    // 2. Update Speed Display (Top Right)
    // Logic: speed is 8.8, so high byte is roughly MPH
    sprintf(buf, "%3d MPH", speed >> 2);
    hud_print(HUD_COL_TIME, HUD_ROW, buf, 11, HUD_COL_BG); // Light Blue for speed
}

void update_countdown_display(uint16_t delay) {
    // delay counts down from 600 to 0
    // 600-481: Clear/Ready
    // 480-421: 3
    // 420-361: 2
    // 360-301: 1
    // 300-241: GO!
    // < 240: Clear

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
    } else if (delay == 240) {
        hud_print(16, 15, "          ", 0, 0);         // Clear row 15
    }
}