#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "player.h"
#include "input.h"
#include "opl.h"

unsigned REDRACER_CONFIG;  // RedRacer Sprite Configuration

static void init_graphics(void)
{
    // Initialize graphics here
    xregn(1, 0, 0, 1, 1); // 320x240 (4:3)

    REDRACER_CONFIG = SPRITE_DATA_END;
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_sprite_t, x_pos_px, startX);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_sprite_t, y_pos_px, startY);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_sprite_t, xram_sprite_ptr, SPRITE_DATA_START);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_sprite_t, log_size, 4); // 16x16
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_sprite_t, has_opacity_metadata, false);

    xregn(1, 0, 1, 5, 4, 0, REDRACER_CONFIG, 1); // Enable RedRacer sprite

}

#define SONG_HZ 60
uint8_t vsync_last = 0;
uint16_t timer_accumulator = 0;
bool music_enabled = true;

int main(void)
{
    puts("Hello from RPMegaRacer!");

    // Enable keyboard input
    xregn(0, 0, 0, 1, KEYBOARD_INPUT);
    // Enable gamepad input
    xregn(0, 0, 2, 1, GAMEPAD_INPUT);
    // Initialize Graphics 
    init_graphics();

    // Initialize OPL
    OPL_Config(1, OPL_ADDR);
    opl_init();

    // Start music playback
    music_init(MUSIC_FILENAME);

    while (1) {
        // --- 1. SYNC TO VSYNC ---
        if (RIA.vsync == vsync_last)
            continue;
        vsync_last = RIA.vsync;

        // --- 2. DRIVE MUSIC ---
        // This math allows any SONG_HZ to work on a 60Hz VSync
        if (music_enabled) {
            timer_accumulator += SONG_HZ;
            while (timer_accumulator >= 60) {
                update_music();
                timer_accumulator -= 60;
            }
        }

        // --- 3. YOUR GAME LOGIC ---
        // Move sprites, check keys, etc.
        // You can safely use RIA.addr0/rw0 here!
    }

}
