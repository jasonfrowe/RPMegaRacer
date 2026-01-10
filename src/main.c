#include <rp6502.h>
#include <stdio.h>
#include <stdbool.h>
#include "constants.h"
#include "player.h"
#include "input.h"
#include "opl.h"
#include "track.h"
#include "sound.h"
#include "ai.h"

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
        0x41A7,
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

    #define NUM_AI_CARS 3
    for (unsigned i = 0; i < NUM_AI_CARS; i++) {
        unsigned config_addr = REDRACER_CONFIG + sizeof(vga_mode4_asprite_t) * (i + 1);
        unsigned sprite_ptr = REDRACER_DATA + ((i + 1) * 0x200); // Each car sprite uses 0x200 bytes (4 tiles)
        
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[0], 256); // SX  (Scale X)
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[1], 0);   // SHY (Shear Y)
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[2], 0);   // TX  (Translate X)
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[3], 0);   // SHX (Shear X)
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[4], 256); // SY  (Scale Y)
        xram0_struct_set(config_addr, vga_mode4_asprite_t, transform[5], 0);   // TY  (Translate Y)

        xram0_struct_set(config_addr, vga_mode4_asprite_t, x_pos_px, (100 + i * 20));
        xram0_struct_set(config_addr, vga_mode4_asprite_t, y_pos_px, (100 + i * 20));
        xram0_struct_set(config_addr, vga_mode4_asprite_t, xram_sprite_ptr, sprite_ptr);
        xram0_struct_set(config_addr, vga_mode4_asprite_t, log_size, 4); // 16x16
        xram0_struct_set(config_addr, vga_mode4_asprite_t, has_opacity_metadata, false);
    }

    xregn(1, 0, 1, 5, 4, 1, REDRACER_CONFIG, (NUM_AI_CARS + 1), 1); // Enable Racer sprite

    // Track map data is loaded via CMakeLists.txt at 0x0850
    TRACK_MAP_START = 0x0850;
    TRACK_MAP_END   = (TRACK_MAP_START + TRACK_MAP_SIZE);

    // Get a copy of the track map in RAM for collision detection
    RIA.addr0 = TRACK_MAP_START;
    RIA.step0 = 1;
    for (unsigned i = 0; i < TRACK_MAP_SIZE; i++) {
        world_map[i] = RIA.rw0;
        // printf("%02X ", world_map[i]);
    }

    TRACK_CONFIG = TRACK_MAP_END;

    int16_t cam_x = 260 - 160; // 100
    int16_t cam_y = 60 - 120;  // -60 (will be clamped to 0)

    if (cam_x < 0) cam_x = 0;
    if (cam_x > 192) cam_x = 192; // (512 - 320)
    if (cam_y < 0) cam_y = 0;
    if (cam_y > 144) cam_y = 144; // (384 - 240)

    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_wrap, false);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_wrap, false);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, cam_x);
    xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, cam_y);
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

// Globals to track camera
int16_t camera_x = 0;
int16_t camera_y = 0;

int16_t next_scroll_x, next_scroll_y;

int main(void)
{
    puts("Hello from RPMegaRacer!");

    // Enable keyboard input
    xregn(0, 0, 0, 1, KEYBOARD_INPUT);
    // Enable gamepad input
    xregn(0, 0, 2, 1, GAMEPAD_INPUT);
    // Initialize Graphics 
    init_player();
    init_ai();  // Initialize AI cars
    init_graphics();
    init_track_physics(); // Initialize terrain collision properties

    // Initialize input mappings (ensure `button_mappings` are set)
    init_input_system(); 

    // Initialize OPL
    OPL_Config(1, OPL_ADDR);
    opl_init();

    // Start music playback
    music_init(MUSIC_FILENAME);

    // Initialize PSG sound
    init_psg();

    while (1) {
        // --- 1. SYNC TO VSYNC ---
        if (RIA.vsync == vsync_last)
            continue;
        vsync_last = RIA.vsync;

        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, next_scroll_x);
        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, next_scroll_y);

        

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

        // Update AI cars
        update_ai();

        // Check collisions
        // check_collisions(&car);

        // Camera system with deadzone - only scroll when car gets near edges
        int16_t car_px_x = car.x >> 8;
        int16_t car_px_y = car.y >> 8;
        
        // Calculate ideal camera offset into the map (for wrapped tilemap)
        // We want car centered, so offset = screen_center - car_position
        int16_t target_offset_x = 160 - car_px_x;
        int16_t target_offset_y = 120 - car_px_y;
        
        // Clamp camera offset to valid map bounds
        // Map is 512x384, screen is 320x240
        // Offset range: x [-192 to 0], y [-144 to 0]
        if (target_offset_x > 0) target_offset_x = 0;          // Don't scroll past left edge
        if (target_offset_x < -192) target_offset_x = -192;    // Don't scroll past right edge (512-320=192)
        if (target_offset_y > 0) target_offset_y = 0;          // Don't scroll past top edge
        if (target_offset_y < -144) target_offset_y = -144;    // Don't scroll past bottom edge (384-240=144)
        
        next_scroll_x = target_offset_x;
        next_scroll_y = target_offset_y;

        // xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, scroll_x);
        // xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, scroll_y);

        // Calculate car's screen position
        int16_t screen_x = car_px_x + next_scroll_x;
        int16_t screen_y = car_px_y + next_scroll_y;

        // Draw player
        draw_player(&car, screen_x, screen_y);

        // Draw AI cars
        draw_ai_cars();

        // printf("Car Pos: (%ld, %ld) Vel:(%d, %d) Angle:%d  \n", car.x >> 8, car.y >> 8, car.vel_x, car.vel_y, car.angle);
        // printf("Camera: (%d, %d) Screen: (%d, %d)        \n", scroll_x, scroll_y, screen_x, screen_y);

    }

}
