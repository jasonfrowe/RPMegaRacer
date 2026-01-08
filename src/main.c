#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "player.h"
#include "input.h"
#include "opl.h"

unsigned REDRACER_CONFIG;   // RedRacer Sprite Configuration
unsigned TRACK_MAP_START;   // Start of track map data in XRAM
unsigned TRACK_MAP_END;     // End of track map data in XRAM
unsigned TRACK_CONFIG;       // Track tilemap configuration

static void init_graphics(void)
{
    // Initialize graphics here
    xregn(1, 0, 0, 1, 1); // 320x240 (4:3)


    // Palette for the tile (16 colors)
    uint16_t tile_palette[16] = {
        0x0020,  // Index 0 (Transparent)
        0x0020,
        0x20A3,
        0x1AE0,
        0x72AA,
        0x0038,
        0x003E,
        0x0372,
        0x2C60,
        0x35AE,
        0x053C,
        0x073E,
        0x93AE,
        0xC4B4,
        0xD534,
        0xF7BE,
    };

    RIA.addr0 = PALETTE_ADDR;
    RIA.step0 = 1;
    for (int i = 0; i < 16; i++) {
        RIA.rw0 = tile_palette[i] & 0xFF;
        RIA.rw0 = tile_palette[i] >> 8;
    }


    REDRACER_CONFIG = SPRITE_DATA_END;

    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[0], 256); // SX  (Scale X)
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[1], 0);   // SHY (Shear Y)
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[2], 0);   // TX  (Translate X)
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[3], 0);   // SHX (Shear X)
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[4], 256); // SY  (Scale Y)
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, transform[5], 0);   // TY  (Translate Y)

    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, x_pos_px, car.x >> 8);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, y_pos_px, car.y >> 8);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, xram_sprite_ptr, REDRACER_DATA);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, log_size, 4); // 16x16
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, has_opacity_metadata, false);

    xregn(1, 0, 1, 5, 4, 1, REDRACER_CONFIG, 1, 1); // Enable RedRacer sprite

    unsigned END_OF_SPRITES = REDRACER_CONFIG + sizeof(vga_mode4_asprite_t);

    // Track map data is loaded vi CMakeLists.txt at 0x10214
    TRACK_MAP_START = END_OF_SPRITES;
    TRACK_MAP_END   = (TRACK_MAP_START + TRACK_MAP_SIZE);

    TRACK_CONFIG = TRACK_MAP_END;

    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_wrap, true);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_wrap, true);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, 0);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, 0);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, width_tiles, TRACK_MAP_WIDTH_TILES);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, height_tiles, TRACK_MAP_HEIGHT_TILES);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, xram_data_ptr, TRACK_MAP_START);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, xram_palette_ptr, PALETTE_ADDR);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, xram_tile_ptr, TRACK_DATA);

    xregn(1, 0, 1, 4, 2, 0x02, TRACK_CONFIG, 0); // Enable sprited tilemap


    printf("Redracer Data at 0x%04X\n", REDRACER_DATA);
    printf("Redracer Config at 0x%04X\n", REDRACER_CONFIG);
    printf("Track Map at 0x%04X - 0x%04X\n", TRACK_MAP_START, TRACK_MAP_END);
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
    init_player();
    init_graphics();

    // Initialize input mappings (ensure `button_mappings` are set)
    init_input_system(); 

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
        // Handle input
        handle_input();

        // Update player
        update_player(&car);

        // Draw player
        draw_player(&car);

    }

}
