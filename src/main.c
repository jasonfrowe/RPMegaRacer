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
#include "hud.h"
#include "racelogic.h"
#include <stdlib.h>

unsigned REDRACER_CONFIG;    // RedRacer Sprite Configuration
unsigned TRACK_MAP_START;    // Start of track map data in XRAM
unsigned TRACK_MAP_END;      // End of track map data in XRAM
unsigned TRACK_CONFIG;       // Track tilemap configuration
unsigned TEXT_CONFIG;        // Text overlay configuration
unsigned text_message_addr;  // Start address for text messages in XRAM
unsigned text_storage_end;   // End address for text messages in XRAM

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

    // Track map offsets
    TRACK_MAP_START = TRACK_MAP_ADDR;
    TRACK_MAP_END   = (TRACK_MAP_START + TRACK_MAP_SIZE);
    
    // NOTE: world_map is now loaded by load_track_data() in init_all_systems()

    TRACK_CONFIG = TRACK_DATA_END;

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

    TEXT_CONFIG = TRACK_CONFIG + sizeof(vga_mode2_config_t);
    text_message_addr = TEXT_CONFIG + sizeof(vga_mode1_config_t);
    const unsigned bytes_per_char = 3; // we write 3 bytes per character into text RAM
    unsigned text_storage_end = text_message_addr + MESSAGE_LENGTH * bytes_per_char;

    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, x_wrap, 0);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, y_wrap, 0);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, x_pos_px, 0); //Bug: first char duplicated if not set to zero
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, y_pos_px, 5);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, width_chars, MESSAGE_WIDTH);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, height_chars, MESSAGE_HEIGHT);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, xram_data_ptr, text_message_addr);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, xram_palette_ptr, 0xFFFF);
    xram0_struct_set(TEXT_CONFIG, vga_mode1_config_t, xram_font_ptr, 0xFFFF);

    // 4 parameters: text mode, 8-bit, config, plane
    xregn(1, 0, 1, 4, 1, 3, TEXT_CONFIG, 2);

    // Clear message buffer to spaces
    for (int i = 0; i < MESSAGE_LENGTH; ++i) message[i] = ' ';

    // Now write the MESSAGE_LENGTH characters into text RAM (3 bytes per char)
    RIA.addr0 = text_message_addr;
    RIA.step0 = 1;
    for (uint16_t i = 0; i < MESSAGE_LENGTH; i++) {
        RIA.rw0 = ' ';
        RIA.rw0 = HUD_COL_WHITE;
        RIA.rw0 = HUD_COL_BG;
    }


    printf("Redracer Data at 0x%04X\n", REDRACER_DATA);
    printf("Redracer Config at 0x%04X\n", REDRACER_CONFIG);
    printf("Track Config at 0x%04X\n", TRACK_CONFIG);
    printf("Text Config at 0x%04X\n", TEXT_CONFIG);
    printf("Text Messages at 0x%04X - 0x%04X\n", text_message_addr, text_storage_end);
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
    init_track_physics(); // Loads track01 map/collision/props
    load_waypoints("tracks/track01/waypoints.bin");
    init_input_system();

    // Audio Setup
    OPL_Config(1, OPL_ADDR);
    opl_init();
    init_opl2_engine_sound(); // Engine sound system
    music_init(MUSIC_FILENAME);
    // init_psg();
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
    // A. FULL PHYSICS: Player vs all 3 AI
    // Keep your heavy resolve_car_collision function for these
    // It should include the wall-checks and sound effects
    for (int i = 0; i < NUM_AI_CARS; i++) {
        resolve_car_collision(&car, &ai_cars[i].car); 
    }

    // B. LIGHT REPULSION: AI vs each other
    // Use the new optimized function that skips wall lookups
    resolve_ai_ai_collision(&ai_cars[0], &ai_cars[1]);
    resolve_ai_ai_collision(&ai_cars[1], &ai_cars[2]);
    resolve_ai_ai_collision(&ai_cars[0], &ai_cars[2]);
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

    uint16_t player_speed = abs(car.vel_x) + abs(car.vel_y);
    hud_refresh_stats(car.laps, player_speed);

}

void debug_draw_waypoints(void) {
    int16_t wx = (waypoints[car.current_waypoint].x + next_scroll_x) >> 3;
    int16_t wy = (waypoints[car.current_waypoint].y + next_scroll_y) >> 3;
    if (wx >= 0 && wx < 40 && wy >= 0 && wy < 30) {
        hud_print(wx, wy, "X", 10, 0); // Green X at the target
    }
}




int main(void) {
    puts("MegaRacer Engine Starting...");
    init_all_systems();
    reset_race();



    while (1) {
        // 1. SYNC
        if (RIA.vsync == vsync_last) continue;
        vsync_last = RIA.vsync;

        // 2. HARDWARE UPDATE (Immediate)
        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, x_pos_px, next_scroll_x);
        xram0_struct_set(TRACK_CONFIG, vga_mode2_config_t, y_pos_px, next_scroll_y);

        // 3. AUDIO
        process_audio_frame();

        // 4. PHYSICS & LOGIC
        handle_input();

        switch (current_state) {
            case STATE_TITLE:
                update_title_screen();
                break;

            case STATE_COUNTDOWN:
                update_race_logic(); // This handles the state_timer--
                update_player(&car);
                update_ai();
                break;

            case STATE_RACING: {
                // Braces {} here fix the "label followed by declaration" warning
                uint16_t player_frame_start_x = car.x;
                uint16_t player_frame_start_y = car.y;

                update_player(&car);
                update_drs_system(&car); // DRS System update

                update_player_progress(); // Updates car.total_progress

                update_ai();

                // Once every 16 frames, recalculate speed for all AI
                if ((RIA.vsync & 15) == 0) {
                    for (int i=0; i < NUM_AI_CARS; i++) {
                        update_ai_rubberbanding(&ai_cars[i]);
                    }
                }

                resolve_all_collisions();

                // Failsafe: check if ramming pushed player into a wall
                if (is_colliding_fast(car.x >> 6, car.y >> 6)) {
                    car.x = player_frame_start_x;
                    car.y = player_frame_start_y;
                    car.vel_x = 0;
                    car.vel_y = 0;
                }

                // Tick the clock
                update_race_timer();
                
                // Draw the clock (You can do this every frame, or every 10 frames to save CPU)
                if ((RIA.vsync & 10) == 0) {
                    hud_draw_timer();
                }

                // finish off countdown if still active
                if (state_timer > 0) {
                    state_timer--; 
                    update_countdown_display(state_timer);
                }

                // Process lap logic
                update_lap_logic(&car, true);
                for (int i = 0; i < NUM_AI_CARS; i++) {
                    update_lap_logic(&ai_cars[i].car, false);
                }

                // --- CHECK FOR WINNER ---
                // Only check if we don't have a winner yet
                if (race_winner == 0xFF) {
                    if (car.laps >= 5) {
                        race_winner = 0; // Player ID
                        stop_engine_sound();
                        current_state = STATE_FINISHED;
                        state_timer = 300;
                    } else {
                        for (uint8_t i = 0; i < NUM_AI_CARS; i++) {
                            if (ai_cars[i].car.laps >= 5) {
                                race_winner = i + 1; // AI ID
                                stop_engine_sound();
                                current_state = STATE_FINISHED;
                                state_timer = 300;
                                break;
                            }
                        }
                    }
                }
            } break;

            case STATE_FINISHED:
                update_finished_screen();
                break;

            case STATE_GAMEOVER:
                // Handle high scores or waiting for reset
                if (is_action_pressed(0, ACTION_PAUSE)) {
                    reset_race();
                }
                break;
        } 

        // 5. POST-PROCESS (Camera & UI)
        update_camera_and_ui();
        hud_draw_drs(&car); 

        // 6. RENDER PREP
        int16_t screen_x = (car.x >> 6) + next_scroll_x;
        int16_t screen_y = (car.y >> 6) + next_scroll_y;
        
        draw_player(&car, screen_x, screen_y);
        draw_ai_cars(next_scroll_x, next_scroll_y);
    }
    return 0;
}
