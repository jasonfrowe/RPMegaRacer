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
#include "collision.h"

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

    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, x_pos_px, car.x >> 6);
    xram0_struct_set(REDRACER_CONFIG, vga_mode4_asprite_t, y_pos_px, car.y >> 6);
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

int16_t next_scroll_x = 0;
int16_t next_scroll_y = 0;

void init_all_systems(void) {
    // Hardware Setup
    xregn(0, 0, 0, 1, KEYBOARD_INPUT);
    xregn(0, 0, 2, 1, GAMEPAD_INPUT);
    
    // Game Logic Setup
    init_player();
    init_ai();
    init_graphics();
    init_track_physics();
    init_input_system();

    // Audio Setup
    OPL_Config(1, OPL_ADDR);
    opl_init();
    music_init(MUSIC_FILENAME);
    init_psg();
}

void process_audio_frame(void) {
    if (!music_enabled) return;
    
    timer_accumulator += SONG_HZ;
    while (timer_accumulator >= 60) {
        update_music();
        timer_accumulator -= 60;
    }
}

void resolve_all_collisions(void) {
    // Player vs AI
    for (int i = 0; i < NUM_AI_CARS; i++) {
        resolve_car_collision(&car, &ai_cars[i].car);
    }
    // AI vs AI
    resolve_car_collision(&ai_cars[0].car, &ai_cars[1].car);
    resolve_car_collision(&ai_cars[0].car, &ai_cars[2].car);
    resolve_car_collision(&ai_cars[1].car, &ai_cars[2].car);
}

void update_camera_and_ui(void) {
    int16_t car_px_x = car.x >> 6;
    int16_t car_px_y = car.y >> 6;

    // Center car, then clamp to map bounds (512x384 map, 320x240 screen)
    int16_t target_x = 160 - car_px_x;
    int16_t target_y = 120 - car_px_y;

    if (target_x > 0) target_x = 0;
    if (target_x < -192) target_x = -192; 
    if (target_y > 0) target_y = 0;
    if (target_y < -144) target_y = -144;

    // Save for the START of the next frame
    next_scroll_x = target_x;
    next_scroll_y = target_y;
}

int main(void) {
    puts("MegaRacer Engine Starting...");
    init_all_systems();

    while (1) {
        // 1. SYNC
        if (RIA.vsync == vsync_last) continue;
        vsync_last = RIA.vsync;

        // // Palette for the tile (16 colors)
        // uint16_t tile_palette[16] = {
        //     0x0020,  // Index 0 (Transparent)
        //     0x0020,
        //     0x41A7,
        //     0x1AE0,
        //     0x72AA,
        //     0x0038,
        //     0x003E,
        //     0x0372,
        //     0x2C60,
        //     0x35AE,
        //     0x053C,
        //     0x073E,
        //     0x93AE,
        //     0xC4B4,
        //     0xD534,
        //     0xF7BE,
        // };

        // RIA.addr0 = PALETTE_ADDR + 2;
        // RIA.step0 = 1;
        // RIA.rw0 = tile_palette[2] & 0xFF;
        // RIA.rw0 = tile_palette[2] >> 8;

        // 2. HARDWARE UPDATE (Do this immediately after VSync!)
        // This sets the scroll for the frame being drawn RIGHT NOW
        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, next_scroll_x);
        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, next_scroll_y);

        // 3. AUDIO
        process_audio_frame();

        uint16_t player_frame_start_x = car.x;
        uint16_t player_frame_start_y = car.y;

        // 4. PHYSICS & LOGIC
        handle_input();
        update_player(&car);
        update_ai();
        resolve_all_collisions();

        // If the ramming phase pushed the player into a wall, 
        // undo the ram and the movement entirely.
        if (is_colliding_fast(car.x >> 6, car.y >> 6)) {
            car.x = player_frame_start_x;
            car.y = player_frame_start_y;
            car.vel_x = 0;
            car.vel_y = 0;
        }

        // Process lap logic
        update_lap_logic(&car, true);
        for (int i = 0; i < NUM_AI_CARS; i++) {
            update_lap_logic(&ai_cars[i].car, false);
        }
        
        // 5. POST-PROCESS (Camera & UI)
        update_camera_and_ui();

        // 6. RENDER PREP
        // Calculate where the cars should be on screen based on the camera we just calculated
        int16_t screen_x = (car.x >> 6) + next_scroll_x;
        int16_t screen_y = (car.y >> 6) + next_scroll_y;
        
        draw_player(&car, screen_x, screen_y);
        draw_ai_cars(next_scroll_x, next_scroll_y); // Passing next_scroll_x/y inside this function is a good idea too


        // RIA.addr0 = PALETTE_ADDR + 2;
        // RIA.step0 = 1;
        // RIA.rw0 = tile_palette[6] & 0xFF;
        // RIA.rw0 = tile_palette[6] >> 8;


    }
    return 0;
}
