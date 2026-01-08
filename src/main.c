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

    xregn(1, 0, 1, 5, 4, 0, REDRACER_CONFIG, 1, 1); // Enable RedRacer sprite

    printf("Sprite Config at 0x%04X\n", REDRACER_CONFIG);
    printf("OPL Config = 0x%X\n", OPL_ADDR);
    printf("GAME_PAD_CONFIG=0x%X\n", GAMEPAD_INPUT);
    printf("KEYBOARD_CONFIG=0x%X\n", KEYBOARD_INPUT);

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

    // Initialize input mappings (ensure `button_mappings` are set)
    init_input_system(); 

    // Initialize OPL
    OPL_Config(1, OPL_ADDR);
    opl_init();

    // Start music playback
    music_init(MUSIC_FILENAME);

    init_player();

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
        // Handle input
        handle_input();

        // Update player
        update_player(&car);

        // Draw player
        draw_player(&car);

    }

}
